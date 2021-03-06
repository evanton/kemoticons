/**********************************************************************************
 *   Copyright (C) 2008 by Carlo Segato <brandon.ml@gmail.com>                    *
 *                                                                                *
 *   This library is free software; you can redistribute it and/or                *
 *   modify it under the terms of the GNU Lesser General Public                   *
 *   License as published by the Free Software Foundation; either                 *
 *   version 2.1 of the License, or (at your option) any later version.           *
 *                                                                                *
 *   This library is distributed in the hope that it will be useful,              *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU            *
 *   Lesser General Public License for more details.                              *
 *                                                                                *
 *   You should have received a copy of the GNU Lesser General Public             *
 *   License along with this library.  If not, see <http://www.gnu.org/licenses/>.*
 *                                                                                *
 **********************************************************************************/

#include "kde_emoticons.h"

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>
#include <QImageReader>

#include <kpluginfactory.h>

K_PLUGIN_FACTORY(KdeEmoticonsFactory, registerPlugin<KdeEmoticons>();)

KdeEmoticons::KdeEmoticons(QObject *parent, const QVariantList &args)
    : KEmoticonsProvider(parent)
{
    Q_UNUSED(args);
}

bool KdeEmoticons::removeEmoticon(const QString &emo)
{
    QString emoticon = QFileInfo(emoticonsMap().key(emo.split(' '))).fileName();
    QDomElement fce = m_themeXml.firstChildElement(QStringLiteral("messaging-emoticon-map"));

    if (fce.isNull()) {
        return false;
    }

    QDomNodeList nl = fce.childNodes();
    for (int i = 0; i < nl.length(); i++) {
        QDomElement de = nl.item(i).toElement();
        if (!de.isNull() && de.tagName() == QLatin1String("emoticon") && (de.attribute(QStringLiteral("file")) == emoticon || de.attribute(QStringLiteral("file")) == QFileInfo(emoticon).baseName())) {
            fce.removeChild(de);
            removeMapItem(emoticonsMap().key(emo.split(' ')));
            removeIndexItem(emoticon, emo.split(' '));
            return true;
        }
    }
    return false;
}

bool KdeEmoticons::addEmoticon(const QString &emo, const QString &text, AddEmoticonOption option)
{
    if (option == Copy) {
        bool result = copyEmoticon(emo);
        if (!result) {
            qWarning() << "There was a problem copying the emoticon";
            return false;
        }
    }

    const QStringList splitted = text.split(' ');
    QDomElement fce = m_themeXml.firstChildElement(QStringLiteral("messaging-emoticon-map"));

    if (fce.isNull()) {
        return false;
    }

    QDomElement emoticon = m_themeXml.createElement(QStringLiteral("emoticon"));
    emoticon.setAttribute(QStringLiteral("file"), QFileInfo(emo).fileName());
    fce.appendChild(emoticon);
    QStringList::const_iterator constIterator;
    for (constIterator = splitted.begin(); constIterator != splitted.end(); ++constIterator) {
        QDomElement emoText = m_themeXml.createElement(QStringLiteral("string"));
        QDomText txt = m_themeXml.createTextNode((*constIterator).trimmed());
        emoText.appendChild(txt);
        emoticon.appendChild(emoText);
    }

    addIndexItem(emo, splitted);
    addMapItem(emo, splitted);
    return true;
}

void KdeEmoticons::saveTheme()
{
    QFile fp(themePath() + '/' + fileName());

    if (!fp.exists()) {
        qWarning() << fp.fileName() << "doesn't exist!";
        return;
    }

    if (!fp.open(QIODevice::WriteOnly)) {
        qWarning() << fp.fileName() << "can't open WriteOnly!";
        return;
    }

    QTextStream emoStream(&fp);
    emoStream.setCodec("UTF-8");
    emoStream << m_themeXml.toString(4);
    fp.close();
}

bool KdeEmoticons::loadTheme(const QString &path)
{
    QFile file(path);

    if (!file.exists()) {
        qWarning() << path << "doesn't exist!";
        return false;
    }

    setThemePath(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << file.fileName() << "can't be open ReadOnly!";
        return false;
    }

    QString error;
    int eli, eco;
    if (!m_themeXml.setContent(&file, &error, &eli, &eco)) {
        qWarning() << file.fileName() << "can't copy to xml!";
        qWarning() << error << "line:" << eli << "column:" << eco;
        file.close();
        return false;
    }

    file.close();

    QDomElement fce = m_themeXml.firstChildElement(QStringLiteral("messaging-emoticon-map"));

    if (fce.isNull()) {
        return false;
    }

    QDomNodeList nl = fce.childNodes();

    clearEmoticonsMap();

    for (int i = 0; i < nl.length(); i++) {
        QDomElement de = nl.item(i).toElement();

        if (!de.isNull() && de.tagName() == QLatin1String("emoticon")) {
            QDomNodeList snl = de.childNodes();
            QStringList sl;

            for (int k = 0; k < snl.length(); k++) {
                QDomElement sde = snl.item(k).toElement();

                if (!sde.isNull() && sde.tagName() == QLatin1String("string")) {
                    sl << sde.text();
                }
            }

            QString emo = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "emoticons/" + themeName() + '/' + de.attribute(QStringLiteral("file")));

            if (emo.isEmpty()) {
                QList<QByteArray> ext = QImageReader::supportedImageFormats();

                for (int j = 0; j < ext.size(); ++j) {
                    emo = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "emoticons/" + themeName() + '/' + de.attribute(QStringLiteral("file")) + '.' + ext.at(j));
                    if (!emo.isEmpty()) {
                        break;
                    }
                }

                if (emo.isEmpty()) {
                    continue;
                }
            }

            addIndexItem(emo, sl);
            addMapItem(emo, sl);
        }
    }

    return true;
}

void KdeEmoticons::newTheme()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/emoticons/" + themeName();
    QDir().mkpath(path);

    QFile fp(path + '/' + "emoticons.xml");

    if (!fp.open(QIODevice::WriteOnly)) {
        qWarning() << fp.fileName() << "can't open WriteOnly!";
        return;
    }

    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\"")));
    doc.appendChild(doc.createElement(QStringLiteral("messaging-emoticon-map")));

    QTextStream emoStream(&fp);
    emoStream.setCodec("UTF-8");
    emoStream << doc.toString(4);
    fp.close();
}

#include "kde_emoticons.moc"

