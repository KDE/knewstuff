/*
    This file is part of KNewStuff2.
    Copyright (c) 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
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
    QCoreApplication::setApplicationName(QStringLiteral("khotnewstuff-dialog"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));

    QCommandLineParser *parser = new QCommandLineParser;
    parser->addHelpOption();
    parser->addPositionalArgument(QStringLiteral("knsrcfile"), i18n("The KNSRC file you want to show. If none is passed, we will use khotnewstuff_test.knsrc, which must be installed."));
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
