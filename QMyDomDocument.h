/*
	Copyright 2010 © Dmitry Philonenko.
	
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. See COPYING file. If not, see <https://www.gnu.org/licenses/>.

  (Это свободная программа: вы можете перераспространять ее и/или изменять
   ее на условиях Стандартной общественной лицензии GNU в том виде, в каком
   она была опубликована Фондом свободного программного обеспечения; либо
   версии 3 лицензии, либо (по вашему выбору) любой более поздней версии.

   Эта программа распространяется в надежде, что она будет полезной,
   но БЕЗО ВСЯКИХ ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА
   или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ. Подробнее см. в Стандартной
   общественной лицензии GNU.

   Вы должны были получить копию Стандартной общественной лицензии GNU
   вместе с этой программой. Если это не так, см.
   <https://www.gnu.org/licenses/>.)
*/

#include <QtXml/QDomDocument>
#include <QMap>

class QMyDomDocument : public QDomDocument {
public:
    QMyDomDocument() : QDomDocument() {}
    QMyDomDocument(const QString& name) : QDomDocument(name) {}
    QMyDomDocument(const QDomDocumentType& doctype) : QDomDocument(doctype) {}

    QDomElement elementById(const QString& id)
    {
        if (map.contains(id)) {
            QDomElement e = map[id];
            if (e.parentNode().nodeType() != QDomNode::BaseNode) {
                return e;
            }

            map.remove(id);
        }

        bool res = this->find(this->documentElement(), id);
        if (res) {
            return map[id];
        }

        return QDomElement();
    }

private:
    QMap<QString, QDomElement> map;

    bool find(QDomElement node, const QString& id)
    {
        if (node.hasAttribute("id")) {
            QString value = node.attribute("id");
            this->map[value] = node;
            if (value == id) {
                return true;
            }
        }

        for (unsigned int i=0; i<node.childNodes().length(); ++i) {
            QDomNode n = node.childNodes().at(i);
            if (n.isElement()) {
                bool res = this->find(n.toElement(), id);
                if (res) {
                    return true;
                }
            }
        }

        return false;
    }
};
