/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "knewstuff2_standard.h"

#include <knewstuff2/engine.h>

#include <KStandardDirs>
#include <QApplication>
#include <KCmdLineArgs>

KNewStuff2Standard::KNewStuff2Standard()
    : QObject()
{
    m_engine = NULL;
}

void KNewStuff2Standard::run(bool upload, bool modal, QString file)
{
    // qCDebug(KNEWSTUFF) << "-- test kns2 engine";

    m_engine = new KNS::Engine();
    bool success = m_engine->init("knewstuff2_test.knsrc");

    // qCDebug(KNEWSTUFF) << "-- engine test result: " << success;

    if (!success) {
        return;
    }

    if (upload) {
        if (modal) {
            // qCDebug(KNEWSTUFF) << "-- start upload (modal)";
            m_engine->uploadDialogModal(file);
            // qCDebug(KNEWSTUFF) << "-- upload (modal) finished";
        } else {
            // qCDebug(KNEWSTUFF) << "-- start upload (non-modal); will not block";
            m_engine->uploadDialog(file);
        }
    } else {
        if (modal) {
            // qCDebug(KNEWSTUFF) << "-- start download (modal)";
            m_engine->downloadDialogModal();
            // qCDebug(KNEWSTUFF) << "-- download (modal) finished";
        } else {
            // qCDebug(KNEWSTUFF) << "-- start download (non-modal); will not block";
            m_engine->downloadDialog();
        }
    }
}

int main(int argc, char **argv)
{
    KCmdLineOptions options;
    options.add("upload <file>", qi18n("Tests upload dialog"));
    options.add("download", qi18n("Tests download dialog"));
    options.add("modal", qi18n("Show modal dialogs"));

    KCmdLineArgs::init(argc, argv, "knewstuff2_standard", 0, qi18n("knewstuff2_standard"), 0);
    KCmdLineArgs::addCmdLineOptions(options);
    QApplication app(KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv());

    // Take source directory into account
    // qCDebug(KNEWSTUFF) << "-- adding source directory " << KNSSRCDIR;
    // qCDebug(KNEWSTUFF) << "-- adding build directory " << KNSBUILDDIR;
    KGlobal::dirs()->addResourceDir("config", KNSSRCDIR);
    KGlobal::dirs()->addResourceDir("config", KNSBUILDDIR);

    KNewStuff2Standard *standard = new KNewStuff2Standard();
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    bool modal = false;
    if (args->isSet("modal")) {
        modal = true;
    }
    if (args->isSet("upload")) {
        standard->run(true, modal, args->getOption("upload"));
    } else if (args->isSet("download")) {
        standard->run(false, modal, QString());
    } else {
        return -1;
    }

    return app.exec();
}

