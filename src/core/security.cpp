/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2004, 2005 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

// app includes
#include "security.h"
#include "question.h"

// qt includes
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <qstandardpaths.h>

#include <QCryptographicHash>

// kde includes
#include <KLocalizedString>

using namespace KNSCore;

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(5, 31)
static QString gpgExecutable()
{
    QString gpgExe = QStandardPaths::findExecutable(QStringLiteral("gpg"));
    if (gpgExe.isEmpty()) {
        gpgExe = QStandardPaths::findExecutable(QStringLiteral("gpg2"));
    }
    if (gpgExe.isEmpty()) {
        return QStringLiteral("gpg");
    }
    return gpgExe;
}

Security::Security()
{
    m_keysRead = false;
    m_gpgRunning = false;
    readKeys();
    readSecretKeys();
}

Security::~Security()
{
}

void Security::readKeys()
{
    if (m_gpgRunning) {
        QTimer::singleShot(5, this, &Security::readKeys);
        return;
    }
    m_runMode = List;
    m_keys.clear();
    m_process = new QProcess();
    QStringList arguments;
    arguments << QStringLiteral("--no-secmem-warning") << QStringLiteral("--no-tty") << QStringLiteral("--with-colon") << QStringLiteral("--list-keys");
    connect(m_process, &QProcess::finished, this, &Security::slotFinished);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &Security::slotReadyReadStandardOutput);
    m_process->start(gpgExecutable(), arguments);
    if (!m_process->waitForStarted()) {
        Q_EMIT signalError(
            i18n("<qt>Cannot start <i>gpg</i> and retrieve the available keys. Make sure that <i>gpg</i> is installed, otherwise verification of downloaded "
                 "resources will not be possible.</qt>"));
        delete m_process;
        m_process = nullptr;
    } else {
        m_gpgRunning = true;
    }
}

void Security::readSecretKeys()
{
    if (m_gpgRunning) {
        QTimer::singleShot(5, this, &Security::readSecretKeys);
        return;
    }
    m_runMode = ListSecret;
    m_process = new QProcess();
    QStringList arguments;
    arguments << QStringLiteral("--no-secmem-warning") << QStringLiteral("--no-tty") << QStringLiteral("--with-colon") << QStringLiteral("--list-secret-keys");
    connect(m_process, &QProcess::finished, this, &Security::slotFinished);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &Security::slotReadyReadStandardOutput);
    m_process->start(gpgExecutable(), arguments);
    if (!m_process->waitForStarted()) {
        delete m_process;
        m_process = nullptr;
    } else {
        m_gpgRunning = true;
    }
}

void Security::slotFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::NormalExit) {
        m_gpgRunning = false;
        delete m_process;
        m_process = nullptr;
        return;
    }
    switch (m_runMode) {
    case ListSecret:
        m_keysRead = true;
        break;
    case Verify:
        Q_EMIT validityResult(m_result);
        break;
    case Sign:
        Q_EMIT fileSigned(m_result);
        break;
    }
    m_gpgRunning = false;
    delete m_process;
    m_process = nullptr;

    Q_UNUSED(exitCode)
}

void Security::slotReadyReadStandardOutput()
{
    QString data;
    while (m_process->canReadLine()) {
        data = QString::fromLocal8Bit(m_process->readLine());
        switch (m_runMode) {
        case List:
        case ListSecret:
            if (data.startsWith(QLatin1String("pub")) || data.startsWith(QLatin1String("sec"))) {
                KeyStruct key;
                if (data.startsWith(QLatin1String("pub"))) {
                    key.secret = false;
                } else {
                    key.secret = true;
                }
                QStringList line = data.split(QLatin1Char(':'), Qt::KeepEmptyParts);
                key.id = line[4];
                QString shortId = key.id.right(8);
                QString trustStr = line[1];
                key.trusted = false;
                if (trustStr == QLatin1Char('u') || trustStr == QLatin1Char('f')) {
                    key.trusted = true;
                }
                data = line[9];
                key.mail = data.section(QLatin1Char('<'), -1, -1);
                key.mail.chop(1);
                key.name = data.section(QLatin1Char('<'), 0, 0);
                if (key.name.contains(QLatin1Char('('))) {
                    key.name = key.name.section(QLatin1Char('('), 0, 0);
                }
                m_keys[shortId] = key;
            }
            break;
        case Verify:
            data = data.section(QLatin1Char(']'), 1, -1).trimmed();
            if (data.startsWith(QLatin1String("GOODSIG"))) {
                m_result &= SIGNED_BAD_CLEAR;
                m_result |= SIGNED_OK;
                QString id = data.section(QLatin1Char(' '), 1, 1).right(8);
                if (!m_keys.contains(id)) {
                    m_result |= UNKNOWN;
                } else {
                    m_signatureKey = m_keys[id];
                }
            } else if (data.startsWith(QLatin1String("NO_PUBKEY"))) {
                m_result &= SIGNED_BAD_CLEAR;
                m_result |= UNKNOWN;
            } else if (data.startsWith(QLatin1String("BADSIG"))) {
                m_result |= SIGNED_BAD;
                QString id = data.section(QLatin1Char(' '), 1, 1).right(8);
                if (!m_keys.contains(id)) {
                    m_result |= UNKNOWN;
                } else {
                    m_signatureKey = m_keys[id];
                }
            } else if (data.startsWith(QLatin1String("TRUST_ULTIMATE"))) {
                m_result &= SIGNED_BAD_CLEAR;
                m_result |= TRUSTED;
            }
            break;

        case Sign:
            if (data.contains(QLatin1String("passphrase.enter"))) {
                KeyStruct key = m_keys[m_secretKey];
                Question question(Question::PasswordQuestion);
                question.setQuestion(
                    i18n("<qt>Enter passphrase for key <b>0x%1</b>, belonging to<br /><i>%2&lt;%3&gt;</i><br />:</qt>", m_secretKey, key.name, key.mail));
                if (question.ask() == Question::ContinueResponse) {
                    m_process->write(question.response().toLocal8Bit() + '\n');
                } else {
                    m_result |= BAD_PASSPHRASE;
                    m_process->kill();
                    return;
                }
            } else if (data.contains(QLatin1String("BAD_PASSPHRASE"))) {
                m_result |= BAD_PASSPHRASE;
            }
            break;
        }
    }
}

void Security::checkValidity(const QString &filename)
{
    m_fileName = filename;
    slotCheckValidity();
}

void Security::slotCheckValidity()
{
    if (!m_keysRead || m_gpgRunning) {
        QTimer::singleShot(5, this, &Security::slotCheckValidity);
        return;
    }
    if (m_keys.isEmpty()) {
        Q_EMIT validityResult(-1);
        return;
    }

    m_result = 0;
    m_runMode = Verify;
    QFileInfo f(m_fileName);
    // check the MD5 sum
    QString md5sum;
    QCryptographicHash context(QCryptographicHash::Md5);
    QFile file(m_fileName);
    if (!m_fileName.isEmpty() && file.open(QIODevice::ReadOnly)) {
        context.reset();
        context.addData(&file);
        md5sum = QString::fromLatin1(context.result().toHex());
        file.close();
    }
    file.setFileName(f.path() + QStringLiteral("/md5sum"));
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray md5sum_file;
        file.readLine(md5sum_file.data(), 50);
        if (!md5sum_file.isEmpty() && QString::fromLatin1(md5sum_file).startsWith(md5sum)) {
            m_result |= MD5_OK;
        }
        file.close();
    }
    m_result |= SIGNED_BAD;
    m_signatureKey.id = QLatin1String("");
    m_signatureKey.name = QLatin1String("");
    m_signatureKey.mail = QLatin1String("");
    m_signatureKey.trusted = false;

    // verify the signature
    m_process = new QProcess();
    QStringList arguments;
    arguments << QStringLiteral("--no-secmem-warning") << QStringLiteral("--status-fd=2") << QStringLiteral("--command-fd=0") << QStringLiteral("--verify")
              << f.path() + QStringLiteral("/signature") << m_fileName;
    connect(m_process, &QProcess::finished, this, &Security::slotFinished);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &Security::slotReadyReadStandardOutput);
    m_process->start(gpgExecutable(), arguments);
    if (m_process->waitForStarted()) {
        m_gpgRunning = true;
    } else {
        Q_EMIT signalError(
            i18n("<qt>Cannot start <i>gpg</i> and check the validity of the file. Make sure that <i>gpg</i> is installed, otherwise verification of downloaded "
                 "resources will not be possible.</qt>"));
        Q_EMIT validityResult(0);
        delete m_process;
        m_process = nullptr;
    }
}

void Security::signFile(const QString &fileName)
{
    m_fileName = fileName;
    slotSignFile();
}

void Security::slotSignFile()
{
    if (!m_keysRead || m_gpgRunning) {
        QTimer::singleShot(5, this, &Security::slotSignFile);
        return;
    }

    QStringList secretKeys;
    for (QMap<QString, KeyStruct>::Iterator it = m_keys.begin(); it != m_keys.end(); ++it) {
        if (it.value().secret) {
            secretKeys.append(it.key());
        }
    }

    if (secretKeys.isEmpty()) {
        Q_EMIT fileSigned(-1);
        return;
    }

    m_result = 0;
    QFileInfo f(m_fileName);

    // create the MD5 sum
    QString md5sum;
    QCryptographicHash context(QCryptographicHash::Md5);
    QFile file(m_fileName);
    if (file.open(QIODevice::ReadOnly)) {
        context.reset();
        context.addData(&file);
        md5sum = QString::fromLatin1(context.result().toHex());
        file.close();
    }
    file.setFileName(f.path() + QStringLiteral("/md5sum"));
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << md5sum;
        m_result |= MD5_OK;
        file.close();
    }

    if (secretKeys.count() > 1) {
        Question question(Question::SelectFromListQuestion);
        question.setQuestion(i18n("Key used for signing:"));
        question.setTitle(i18n("Select Signing Key"));
        question.setList(secretKeys);
        if (question.ask() == Question::OKResponse) {
            m_secretKey = question.response();
        } else {
            // emit an error to be forwarded to the user for selecting a signing key...
            Q_EMIT fileSigned(0);
            return;
        }
    } else {
        m_secretKey = secretKeys[0];
    }

    // verify the signature
    m_process = new QProcess();
    QStringList arguments;
    arguments << QStringLiteral("--no-secmem-warning") << QStringLiteral("--status-fd=2") << QStringLiteral("--command-fd=0") << QStringLiteral("--no-tty")
              << QStringLiteral("--detach-sign") << QStringLiteral("-u") << m_secretKey << QStringLiteral("-o") << f.path() + QStringLiteral("/signature")
              << m_fileName;
    connect(m_process, &QProcess::finished, this, &Security::slotFinished);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &Security::slotReadyReadStandardOutput);
    m_runMode = Sign;
    m_process->start(gpgExecutable(), arguments);
    if (m_process->waitForStarted()) {
        m_gpgRunning = true;
    } else {
        Q_EMIT signalError(
            i18n("<qt>Cannot start <i>gpg</i> and sign the file. Make sure that <i>gpg</i> is installed, otherwise signing of the resources will not be "
                 "possible.</qt>"));
        Q_EMIT fileSigned(0);
        delete m_process;
        m_process = nullptr;
    }
}

#include "moc_security.cpp"

#endif
