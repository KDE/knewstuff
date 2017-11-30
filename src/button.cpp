/*
    knewstuff3/ui/knewstuffbutton.cpp.
    Copyright (c) 2004 Aaron J. Seigo <aseigo@kde.org>

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

#include "button.h"

#include <klocalizedstring.h>
#include <kauthorized.h>
#include <kmessagebox.h>
#include "downloaddialog.h"
#include "ui/widgetquestionlistener.h"

#include <QPointer>

namespace KNS3
{
class ButtonPrivate
{
public:
    QString configFile;
};

Button::Button(const QString &text,
               const QString &configFile,
               QWidget *parent)
    : QPushButton(parent),
      d(new ButtonPrivate)
{
    setButtonText(text);
    d->configFile = configFile;
    init();
}

Button::Button(QWidget *parent)
    : QPushButton(parent),
      d(new ButtonPrivate)
{
    setButtonText(i18n("Download New Stuff..."));
    init();
}

Button::~Button()
{
    delete d;
}

void Button::init()
{
    const bool authorized = KAuthorized::authorize(QStringLiteral("ghns"));
    if (!authorized) {
        setEnabled(false);
        setVisible(false);
    }

    setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));
    connect(this, &QAbstractButton::clicked, this, &Button::showDialog);
    WidgetQuestionListener::instance();
}

void Button::setButtonText(const QString &what)
{
    setText(what);
}

void Button::setConfigFile(const QString &configFile)
{
    d->configFile = configFile;
}

void Button::showDialog()
{
    if (!KAuthorized::authorize(QStringLiteral("ghns"))) {
        KMessageBox::information(this, QStringLiteral("Get Hot New Stuff is disabled by the administrator"), QStringLiteral("Get Hot New Stuff disabled"));
        return;
    }
    emit aboutToShowDialog();

    QPointer<DownloadDialog> dialog = new DownloadDialog(d->configFile, this);
    dialog->exec();

    if (dialog)
        emit dialogFinished(dialog->changedEntries());

    delete dialog;
}

}

