/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "knsrcmodel.h"

#include "engine.h"

#include <KLocalizedString>

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("knewstuff-dialog"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    KLocalizedString::setApplicationDomain("knewstuff-dialog");

    QCommandLineParser *parser = new QCommandLineParser;
    parser->addHelpOption();
    parser->addPositionalArgument(QStringLiteral("knsrcfile"), i18n("The KNSRC file you want to show. If none is passed, you will be presented with a dialog which lets you switch between all the config files installed into the systemwide knsrc file location, which on your system is: %1", KNSCore::Engine::configSearchLocations().last()));
    parser->process(app);

    QQmlApplicationEngine *appengine = new QQmlApplicationEngine();
    qmlRegisterType<KNSRCModel>("org.kde.newstuff.tools.dialog", 1, 0, "KNSRCModel");
    appengine->rootContext()->setContextProperty(QLatin1String("knsrcFilesLocation"), KNSCore::Engine::configSearchLocations().last());

    if (parser->positionalArguments().count() > 0) {
        appengine->rootContext()->setContextProperty(QLatin1String("knsrcfile"), parser->positionalArguments().first());
        appengine->load(QStringLiteral("qrc:/qml/dialog.qml"));
    } else {
        appengine->load(QStringLiteral("qrc:/qml/main.qml"));
    }

    return app.exec();
}
