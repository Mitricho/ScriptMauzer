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
/*---------------------------------------------------------------------------------------*/
#include <QFile>
#include <QColor>
#include <QRegExp>
#include <QRgb>
#include <QtCore>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTextCodec>
#include <QDebug>
#include <QSize>
#include <QStringList>
#include <QDomDocument>
#include "QMyDomDocument.h"
#include <QCoreApplication>
#include <QScriptEngine>
#include <QScriptValue>
#include <QSettings>
#include <QLibraryInfo>
#include <QScriptValueIterator>
#include <QStringBuilder>
#include <QImage>
#include <QProcess>

#include <errno.h>
#include <stdio.h>
#include <windows.h>

#define LODWORD(l)   ((DWORD)((DWORDLONG)(l)))

namespace anima
{
/*---------------------------------------------------------------------------------------*/
    QMyDomDocument doc;
/*---------------------------------------------------------------------------------------*/
    QString slh(QLatin1String("/"));
    QSettings settings("Atech","Fabrika");//,QSettings::IniFormat

    QDir::Filters AllDirsAndFiles = QDir::Dirs |
            QDir::Files |
            QDir::NoSymLinks |
            QDir::NoDotAndDotDot |
            QDir::NoDot |
            QDir::NoDotDot |
            QDir::Readable;

    QDir::Filters FilesOnly = QDir::Files |
            QDir::NoSymLinks |
            QDir::NoDotAndDotDot |
            QDir::NoDot |
            QDir::NoDotDot |
            QDir::Readable;

    QDir::Filters DirsOnly = QDir::Dirs |
            QDir::NoSymLinks |
            QDir::NoDotAndDotDot |
            QDir::NoDot |
            QDir::NoDotDot |
            QDir::Readable;

/*---------------------------------------------------------------------------------------*/
QString getFileVersion(QString path)
{
    DWORD dwHandle, dwLen;
    UINT BufLen;
    LPTSTR lpData;
    LPTSTR lpBuffer;
    LPTSTR LibName = TEXT("");
    VS_FIXEDFILEINFO *pFileInfo;

    LibName = new wchar_t[path.size()+1];

    path.toWCharArray(LibName);

    dwLen = GetFileVersionInfoSize (LibName, &dwHandle);

    qDebug() << "---------------------------------";
    qDebug() << "Get file version: " << path;

    if (!dwLen)	{
        qDebug() << "VersionInfo not found";
        return QString();
    }
    lpData = (LPTSTR) malloc (dwLen);
    if (!lpData) {
        qDebug() <<  "malloc error";
        return QString();
    }
    if (!GetFileVersionInfo (LibName, dwHandle, dwLen, lpData)) {
        free (lpData);
        qDebug() << "VersionInfo: not found";
        return QString();
    }
    if (!VerQueryValueW(lpData,
                        TEXT("\\"),
                        (LPVOID*)&pFileInfo,
                        (PUINT) &BufLen))
    {
        //qDebug() << "VersionInfo: not found";
    }
    else {
        printf ("MajorVersion:         %d\n", HIWORD(pFileInfo->dwFileVersionMS));
        printf ("MinorVersion:         %d\n", LOWORD(pFileInfo->dwFileVersionMS));
        printf ("BuildNumber:          %d\n", HIWORD(pFileInfo->dwFileVersionLS));
        printf ("RevisionNumber (QFE): %d\n", LOWORD(pFileInfo->dwFileVersionLS));
    }

    /* language ID 040904E4: U.S. English, char set = Windows, Multilingual */
    if (!VerQueryValueW(lpData,
                        TEXT("\\StringFileInfo\\040904E4\\FileVersion"),
                        (LPVOID*)&lpBuffer,
                        (PUINT)&BufLen))
    {
        qDebug() << "FileVersion: not found";
    }
    else{
        qDebug() << "FileVersion:" << QString::fromWCharArray(lpBuffer);
    }

    if (!VerQueryValueW(lpData,
                        TEXT("\\StringFileInfo\\040904E4\\PrivateBuild"),
                        (LPVOID*)&lpBuffer,
                        (PUINT) &BufLen))
    {
        qDebug() << "PrivateBuild: not found";
    }
    else{
        qDebug() << "PrivateBuild:" << QString::fromWCharArray(lpBuffer);
    }

    if (!VerQueryValueW(lpData,
                        TEXT("\\StringFileInfo\\040904E4\\LibToolFileVersion"),
                        (LPVOID*) &lpBuffer,
                        (PUINT) &BufLen))
    {
        qDebug() << "LibToolFileVersion: not found";
    }
    else{
        qDebug() << "LibToolFileVersion:" << QString::fromWCharArray(lpBuffer);
    }
    if (!VerQueryValueW(lpData,
                        TEXT("\\StringFileInfo\\040904E4\\ProductVersion"),
                        (LPVOID*) &lpBuffer,
                        (PUINT) &BufLen))
    {
        qDebug() << "ProductVersion: not found";
    }
    else
    {
        qDebug() << "ProductVersion:" << QString::fromWCharArray(lpBuffer);
    }

    free (lpData);

    qDebug() << "---------------------------------";
    return QString::fromWCharArray(lpBuffer);
}
/*---------------------------------------------------------------------------------------*/
QString getVersionString(QString fName)
{
    //qDebug() << "getVersionString for " << fName;

    DWORD dwHandle;
    DWORD dwLen = GetFileVersionInfoSize(fName.toStdWString().c_str(), &dwHandle);

    // GetFileVersionInfo
    LPVOID lpData = new BYTE[dwLen];
    if(!GetFileVersionInfo(fName.toStdWString().c_str(), dwHandle, dwLen, lpData)){
        qDebug() << "error in GetFileVersionInfo";
        delete[] lpData;
        return "";
    }

    VS_FIXEDFILEINFO *lpBuffer = NULL;
    UINT uLen;

    if(!VerQueryValue(lpData,QString("\\").toStdWString().c_str(),(LPVOID*)&lpBuffer,&uLen)){
        qDebug() << "error in VerQueryValue";
        delete[] lpData;
        return "";
    }

    /* language ID 040904E4: U.S. English, char set = Windows, Multilingual
    LPTSTR lptstr;
    UINT BufLen;
    if (!VerQueryValueW(lpData,
                        TEXT("\\StringFileInfo\\040904E4\\ProductVersion"),
                        (LPVOID*)&lptstr,
                        (PUINT)&BufLen))
    {
        qDebug() << "FileVersion: not found";
    }
    else{
        qDebug() << "FileVersion:" << QString::fromWCharArray(lptstr);
    }*/


    return QString::number( (lpBuffer->dwFileVersionMS >> 16 ) & 0xffff ) + "." +
    QString::number( ( lpBuffer->dwFileVersionMS) & 0xffff ) + "." +
    QString::number( ( lpBuffer->dwFileVersionLS >> 16 ) & 0xffff ) + "." +
    QString::number( ( lpBuffer->dwFileVersionLS) & 0xffff );
}
/*---------------------------------------------------------------------------------------*/
QScriptValue getFileVer(QScriptContext *context, QScriptEngine *engine){
    if(context->argumentCount() > 0)
    {
        QString path = context->argument(0).toString();
        return engine->toScriptValue(getVersionString(path));
    }
    else
    {
        return engine->toScriptValue(QString());
    }
}
/*---------------------------------------------------------------------------------------*/
    QScriptValue btoa(QScriptContext *context, QScriptEngine *engine){
        if(context->argumentCount() > 0){
            if(context->argument(0).isString())
            {
                QString data = context->argument(0).toString();
                QByteArray b = data.toUtf8();
                return engine->toScriptValue(QString(b.toBase64()));
            }
            else{
                return engine->toScriptValue(QString());
            }
        }else{
            return engine->toScriptValue(QString());
        }
    }
/*---------------------------------------------------------------------------------------*/
    const char* stringToChar(const QString &str){
        QByteArray barr = str.toLocal8Bit();
        const char *swp = barr.data();
        return swp;
    }
/*---------------------------------------------------------------------------------------*/
    bool writeFile(const QString &path, QString textdata, QString codecName)
    {
        QFile file(path);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
            return false;
        }
        QTextStream out(&file);
        out.setGenerateByteOrderMark(true);
        out.setCodec(stringToChar(codecName));

        QRegExp ex("\r");
        textdata.replace(ex,QString(""));

        out << textdata;
        out.flush();
        file.close();
        return true;
    }
/*--------------------------------------------------------------------------------------*/
    QString cleanStr(const QString &str,bool isFilename = false, bool isPath = false)
    {
        QString clKey = str.simplified().trimmed(); //Qt::CaseInsensitive
        QString r("");
        clKey.replace(QString("{"),r);
        clKey.replace(QString("}"),r);
        clKey.replace(QString("#"),r);
        clKey.replace(QString("!"),r);
        clKey.replace(QString("~"),r);
        clKey.replace(QString("("),r);
        clKey.replace(QString(")"),r);
        clKey.replace(QString("+"),r);
        clKey.replace(QString(":"),r);
        if(!isPath)clKey.replace(QString("/"),r);
        if(!isPath)clKey.replace(QString("\\"),r);
        clKey.replace(QString("%"),r);
        clKey.replace(QString("$"),r);
        clKey.replace(QString("@"),r);
        clKey.replace(QString("^"),r);
        clKey.replace(QString("&"),r);
        clKey.replace(QString("?"),r);
        clKey.replace(QString("*"),r);
        clKey.replace(QString("|"),r);
        clKey.replace(QString("["),r);
        clKey.replace(QString("]"),r);
        clKey.replace(QString(";"),r);
        clKey.replace(QString("<"),r);
        clKey.replace(QString(">"),r);
        clKey.replace(QString("`"),r);
        clKey.replace(QString("="),r);
        clKey.replace(QString("?"),r);
        clKey.replace(QString("'"),r);
        clKey.replace(QString("\""),r);
        clKey.replace(QString("№").toLatin1(),r);
        clKey.replace(QString(":"),r);
        clKey.replace(QString(","),r);
        if(isFilename){
            //clKey.replace(QString("-"),QString("-"));
            clKey.replace(QString(" "),QString("-"));
        }
        return clKey;
    }
/*---------------------------------------------------------------------------------------*/
    QString translit(QString ret)
    {
        //qDebug() << QString("Transliterate str: %1").arg(ret);
        ret.replace(QString("а"),QString("a"), Qt::CaseSensitive);
        ret.replace(QString("б"),QString("b"),Qt::CaseSensitive);
        ret.replace(QString("в"),QString("v"),Qt::CaseSensitive);
        ret.replace(QString("г"),QString("g"),Qt::CaseSensitive);
        ret.replace(QString("д"),QString("d"),Qt::CaseSensitive);
        ret.replace(QString("е"),QString("e"),Qt::CaseSensitive);
        ret.replace(QString("ё"),QString("yo"),Qt::CaseSensitive);
        ret.replace(QString("ж"),QString("zh"),Qt::CaseSensitive);
        ret.replace(QString("з"),QString("z"),Qt::CaseSensitive);
        ret.replace(QString("й"),QString("i"),Qt::CaseSensitive);
        ret.replace(QString("и"),QString("i"),Qt::CaseSensitive);
        ret.replace(QString("к"),QString("k"),Qt::CaseSensitive);
        ret.replace(QString("л"),QString("l"),Qt::CaseSensitive);
        ret.replace(QString("м"),QString("m"),Qt::CaseSensitive);
        ret.replace(QString("н"),QString("n"),Qt::CaseSensitive);
        ret.replace(QString("о"),QString("o"),Qt::CaseSensitive);
        ret.replace(QString("п"),QString("p"),Qt::CaseSensitive);
        ret.replace(QString("р"),QString("r"),Qt::CaseSensitive);
        ret.replace(QString("с"),QString("s"),Qt::CaseSensitive);
        ret.replace(QString("т"),QString("t"),Qt::CaseSensitive);
        ret.replace(QString("у"),QString("u"),Qt::CaseSensitive);
        ret.replace(QString("ф"),QString("f"),Qt::CaseSensitive);
        ret.replace(QString("х"),QString("ch"),Qt::CaseSensitive);
        ret.replace(QString("ц"),QString("z"),Qt::CaseSensitive);
        ret.replace(QString("ч"),QString("ch"),Qt::CaseSensitive);
        ret.replace(QString("ш"),QString("sh"),Qt::CaseSensitive);
        ret.replace(QString("щ"),QString("ch"),Qt::CaseSensitive);
        ret.replace(QString("ъ"),QString(""),Qt::CaseSensitive);
        ret.replace(QString("ы"),QString("y"),Qt::CaseSensitive);
        ret.replace(QString("ь"),QString(""),Qt::CaseSensitive);
        ret.replace(QString("э"),QString("ye"),Qt::CaseSensitive);
        ret.replace(QString("ю"),QString("yu"),Qt::CaseSensitive);
        ret.replace(QString("я"),QString("ya"),Qt::CaseSensitive);

        ret.replace("А","A",Qt::CaseSensitive);
        ret.replace("Б","B",Qt::CaseSensitive);
        ret.replace("В","V",Qt::CaseSensitive);
        ret.replace("Г","G",Qt::CaseSensitive);
        ret.replace("Д","D",Qt::CaseSensitive);
        ret.replace("Е","E",Qt::CaseSensitive);
        ret.replace("Ё","YE",Qt::CaseSensitive);
        ret.replace("Ж","ZH",Qt::CaseSensitive);
        ret.replace("З","Z",Qt::CaseSensitive);
        ret.replace("Й","I",Qt::CaseSensitive);
        ret.replace("И","I",Qt::CaseSensitive);
        ret.replace("К","K",Qt::CaseSensitive);
        ret.replace("Л","L",Qt::CaseSensitive);
        ret.replace("М","M",Qt::CaseSensitive);
        ret.replace("Н","N",Qt::CaseSensitive);
        ret.replace("О","O",Qt::CaseSensitive);
        ret.replace("П","P",Qt::CaseSensitive);
        ret.replace("Р","R",Qt::CaseSensitive);
        ret.replace("С","S",Qt::CaseSensitive);
        ret.replace("Т","T",Qt::CaseSensitive);
        ret.replace("У","U",Qt::CaseSensitive);
        ret.replace("Ф","F",Qt::CaseSensitive);
        ret.replace("Х","CH",Qt::CaseSensitive);
        ret.replace("Ц","Z",Qt::CaseSensitive);
        ret.replace("Ч","CH",Qt::CaseSensitive);
        ret.replace("Ш","SH",Qt::CaseSensitive);
        ret.replace("Щ","CH",Qt::CaseSensitive);
        ret.replace("Ъ","",Qt::CaseSensitive);
        ret.replace("Ы","Y",Qt::CaseSensitive);
        ret.replace("Ь","",Qt::CaseSensitive);
        ret.replace("Э","YE",Qt::CaseSensitive);
        ret.replace("Ю","YU",Qt::CaseSensitive);
        ret.replace("Я","YA",Qt::CaseSensitive);

        //qDebug() << QString("translit string: %1").arg(ret);

        return ret;
    }
/*---------------------------------------------------------------------------------------*/
    QString validatePath(const QString &loadFile, QScriptEngine *engine)
    {
        QString currentFileName = engine->globalObject().property("qs").property("script").property("absoluteFilePath").toString();
        QUrl url(currentFileName);
        QUrl path = url.resolved(loadFile);
        return path.toString();
    }
/*---------------------------------------------------------------------------------------*/
    QString newFileExt(const QString &path, const QString &newExt){
        if(!path.isEmpty() && !newExt.isEmpty()){
            QFileInfo fi(path);
            QDir folder = fi.absoluteDir();
            QString parentDir = folder.absolutePath() % slh;
            if(!QFileInfo(parentDir).exists())folder.mkpath(parentDir);
            return parentDir % fi.baseName() % "." % newExt;
        }else{
            qDebug() << "Error: newFileExt: midding arguments";
            return QString();
        }
    }
/*---------------------------------------------------------------------------------------*/
    QScriptValue imageColor(QScriptContext *context, QScriptEngine *engine)
    {
        /*  Использование из JavaScript:
            imageColor(path, xPos="left",yPos="top");
            либо:
            imageColor(path, xPos=20,yPos=38);*/

        QString def("#000000");
        if(context->argumentCount() > 0){
            if(context->argument(0).isString())
            {
                QString path = context->argument(0).toString();
                QString loadFile = validatePath(path, engine);
                if(QFileInfo(loadFile).exists())
                {
                    QImage pic;
                    if(pic.load(loadFile))
                    {
                       if(context->argumentCount() > 2)
                       {
                           int ix = 0;
                           int yg = 0;
                           if(context->argument(1).isString() &&
                              context->argument(2).isString())
                           {
                               QString xPos = context->argument(1).toString();
                               QString yPos = context->argument(2).toString();

                               if(xPos.toLower() == "left"){
                                   ix = 0;
                               }else if(xPos.toLower() == "center"){
                                   ix = qRound(qreal(pic.width())/qreal(2));
                               }else if(xPos.toLower() == "right"){
                                   ix = pic.width() - 1;
                               }

                               if(yPos.toLower() == "top"){
                                   yg = 0;
                               }else if(yPos.toLower() == "middle"){
                                   yg = qRound(qreal(pic.height())/qreal(2));
                               }else if(yPos.toLower() == "bottom"){
                                   yg = pic.height() - 1;
                               }
                           }else if(context->argument(1).isNumber() &&
                                    context->argument(2).isNumber())
                           {
                               ix = int(context->argument(1).toNumber());
                               yg = int(context->argument(2).toNumber());
                           }

                           if(ix >= 0 && ix < pic.width() && yg < pic.height() && yg >= 0){
                              QPoint p(ix,yg);
                              QRgb color = pic.pixel(p);
                              QColor c(color);
                              return engine->toScriptValue(c.name());
                           }else{
                               qDebug() << QString("imageColor: Out of range: x:%1, y:%2").arg(ix).arg(yg);
                           }
                       }
                    }else{
                        qDebug() << QString("imageColor: Failed to load %1").arg(loadFile);
                        return engine->toScriptValue(def);
                    }
                }else{
                qDebug() << QString("imageColor: File not found %1").arg(loadFile);
                return engine->toScriptValue(def);
            }
            }
        }
        qDebug() << "imageColor: Bad arguments. Returning default color";
        return engine->toScriptValue(def);
    }
/*---------------------------------------------------------------------------------------*/
    QScriptValue getSumForFile(QScriptContext *context, QScriptEngine *engine){
        if(context->argumentCount() > 0)
        {
            QString path = context->argument(0).toString();
            QString loadFile = validatePath(path, engine);
            QFile theFile(loadFile);
            QByteArray thisFile;
            if (theFile.open(QIODevice::ReadOnly))
            {
                thisFile = theFile.readAll();
            }
            else
            {
                qDebug() << "Failed to open " << loadFile;
            }
            QByteArray ba = QCryptographicHash::hash((thisFile), QCryptographicHash::Md5).toHex();
            char *c_str2 = ba.data();
            return engine->toScriptValue(QString(c_str2));
        }
        else
        {
            return engine->toScriptValue(QString());
        }
    }
/*---------------------------------------------------------------------------------------*/
    QScriptValue processImage(QScriptContext *context, QScriptEngine *engine)
    {
        //processImage(loadFile, width=-1,height=-1, crop=false, format='PNG', removeSourceFile, outFile);

        if(context->argumentCount() > 0)
        {
            QString path = context->argument(0).toString();
            QString loadFile = validatePath(path, engine);
            if(context->argumentCount() > 2)
            {
                int quality = -1;
                QString format = "PNG";
                QString outFile;
                QImage result;
                bool crop = false;
                qsreal width  = -1;
                qsreal height = -1;
                if(context->argument(1).isNumber() &&
                   context->argument(2).isNumber())
                {
                    width = context->argument(1).toNumber();
                    height = context->argument(2).toNumber();
                }
                if(context->argumentCount() > 3){
                    if(context->argument(3).isBool()){
                        crop = context->argument(3).toBool();
                    }
                }
                QImage pic(loadFile);
                if((pic.width() != width || pic.height() != height) &&
                    width != -1 && height != -1)
                {
                    if(crop && (pic.width() > width || pic.height() > height)){
                        width  = qMin(pic.width(),int(width));
                        height = qMin(pic.height(),int(height));
                        result = pic.copy(0,0,width,height);
                    }else{
                        width  = qMax(pic.width(),int(width));
                        height = qMax(pic.height(),int(height));
                        result = pic.scaled(QSize(width,height),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
                    }
                }else{
                    result = pic;
                }
                if(!result.isNull()){
                    if(context->argumentCount() > 4){
                        if(context->argument(4).isString()){
                            format = context->argument(4).toString();
                        }
                    }
                    const char* ext;
                    if(format.toUpper() == "PNG"){
                       ext = "png";
                    }else if(format.toUpper() == "JPG" || format.toUpper() == "JPEG"){
                       ext = "jpg";
                       quality = 80;
                    }else if(format.toUpper() == "BMP"){
                       ext = "bmp";
                    }else if(format.toUpper() == "TIFF"){
                       ext = "tiff";
                    }else{
                       ext = "png";
                       qDebug() << "Setting default ext 'PNG'";
                    }

                    if(context->argumentCount() > 5){
                        if(context->argument(5).isBool()){
                            bool removeSourceFile = context->argument(5).toBool();
                            if(removeSourceFile){
                                pic = QImage();
                                //qDebug() << "loadFile=" + loadFile;
                                QFileInfo fi(loadFile);
                                if(fi.exists()){
                                    QFile f(loadFile);
                                    bool deleted = f.remove();
                                    if(!deleted){
                                        qDebug() << QString("Failed to delete image '%1' as was requested").arg(loadFile);
                                    }
                                }else{
                                    qDebug() << QString("Image file '%1' not found").arg(loadFile);
                                }
                            }
                        }
                    }
                    if(context->argumentCount() > 6){
                        if(context->argument(6).isString()){
                            QString p = context->argument(6).toString();
                            outFile = validatePath(p, engine);
                        }
                    }else{
                        QString ex = QString(ext);
                        if(QFileInfo(loadFile).suffix() != ex){
                            outFile = newFileExt(loadFile,ex);
                        }else{
                            outFile = loadFile.append(".").append(ext);
                        }
                    }
                    //qDebug() << QString("Saving file as '%1'").arg(outFile);
                    result.save(outFile,ext,quality);
                }else{
                    qDebug() << QString("Failed to process image '%1'").arg(loadFile);
                }
            }
        }
        return engine->toScriptValue(false);
    }
/*---------------------------------------------------------------------------------------*/
    QString _readTextFile(QString path)
    {
        QString contents = "";
        QFile file(path);
        if (!file.open(QFile::ReadOnly)) {
            qDebug() << QString("Failed to load file: %1").arg(path);
        }else{
            QTextStream stream(&file);
            QString line = stream.readAll();
            QByteArray ba = line.toUtf8();
            QTextCodec *codec = QTextCodec::codecForName("UTF-8");
            contents = codec->toUnicode(ba);
            file.close();
            stream.flush();
        }
        return contents;
    }
/*---------------------------------------------------------------------------------------*/
    QString _loadTextFile(QScriptContext *context, QScriptEngine *engine)
    {
        QString contents = "";
        if(context->argumentCount() > 0)
        {
            QString path = context->argument(0).toString();
            QString loadFile = validatePath(path, engine);
            contents = _readTextFile(loadFile);
        }else{
            qDebug() << QString("loadTextFile: Wrong number of arguments: %1").arg(context->argumentCount());
        }
        return contents;
    }
/*---------------------------------------------------------------------------------------*/
    QScriptValue loadTextFile(QScriptContext *context, QScriptEngine *engine)
    {
        QString contents = _loadTextFile(context,engine);

        return engine->toScriptValue(contents);
    }
/*---------------------------------------------------------------------------------------*/
    QScriptValue loadXMLFile(QScriptContext *context, QScriptEngine *engine)
    {
        if(context->argumentCount() > 0)
        {
            QString templateStr = _loadTextFile(context, engine);
            QString errorStr = "";
            int errorLine = 0;
            int errorColumn = 0;
            doc.clear();
            if(!doc.setContent(templateStr.toUtf8(), false, &errorStr, &errorLine, &errorColumn)){
                qDebug() << errorStr % QString(" at line %1, column %2").arg(errorLine).arg(errorColumn);
                engine->toScriptValue(false);
            }
        }else{
            qDebug() << QString("Wrong number of arguments: %1").arg(context->argumentCount());
        }
        return engine->toScriptValue(true);
    }
/*---------------------------------------------------------------------------------------*/
    QScriptValue parseCode(QScriptContext *context, QScriptEngine *engine){
        if(context->argumentCount() > 1){
            if(context->argument(1).isString())
            {
                QString variableName = context->argument(1).toString();
                QScriptValue templte = loadTextFile(context, engine);
                QString codeFileStr = templte.toString();
                QStringList lines = codeFileStr.split(";");
                foreach(QString line, lines){
                    if(line.contains(variableName,Qt::CaseInsensitive) &&
                       line.contains("=",Qt::CaseInsensitive)){
                       QStringList expression = line.split("=");
                       if(expression.length() > 1){
                           QString retval = expression.at(1).simplified().replace("\"","");
                          //qDebug() << QString("var:%1, val:%2").arg(expression.at(0)).arg(retval);
                          return engine->toScriptValue(retval);
                       }
                    }
                }
            }
        }
        return engine->toScriptValue(QString());
    }
/*---------------------------------------------------------------------------------------*/
        QScriptValue applyTemplate(QScriptContext *context, QScriptEngine *engine)
        {
            if(context->argumentCount() > 1){
                QScriptValue templte = loadTextFile(context, engine);
                QScriptValue obj = context->argument(1);
                if(obj.isValid())
                {
                    QScriptValue data = engine->toObject(obj);
                    QString templateStr = templte.toString();

                    if(data.isObject())
                    {
                        QScriptValueIterator it(data);
                        while (it.hasNext()){
                            it.next();
                            QString markStart =  "<" % it.name() % ">";
                            if(it.value().isString()){
                                templateStr.replace(markStart, it.value().toString());
                            }else  if(it.value().isBool())
                            {
                                QString markEnd   =  "</" % it.name() % ">";
                                QString marker = markStart % "*" % markEnd;
                                //qDebug() << "To replace: " % marker;
                                QRegExp rx(marker,Qt::CaseInsensitive,QRegExp::Wildcard);

                                if(!it.value().toBool()){
                                    templateStr.replace(rx, "");
                                    //qDebug() << "Replaced: " % marker;
                                }else{
                                    templateStr.replace(markStart, "");
                                    templateStr.replace(markEnd, "");
                                }
                            }else  if(it.value().isNumber()){
                                qsreal val = it.value().toNumber();
                                templateStr.replace(markStart,QString::number(val));

                            }else{
                                //qDebug() << QString("Property %1 is '%2', not a string or bool").arg(it.name()).arg(QString(it.value().toVariant().typeName()));
                            }
                        }
                        return engine->toScriptValue(templateStr);
                    }else{
                        qDebug() << "Not an object";
                    }
                }else{
                    qDebug() << "Received object is invalid";
                }
            }else{
                qDebug() <<QString("applyTemplate: wrong number of arguments %1. Required 2").arg(context->argumentCount());
            }
            return engine->toScriptValue(QString());
        }
    /*---------------------------------------------------------------------------------------*/
        QScriptValue applyXMLtemplate(QScriptContext *context, QScriptEngine *engine)
        {
            if(context->argumentCount() > 1)
            {
                loadXMLFile(context, engine);
                if(!doc.isNull())
                {
                    QScriptValue obj = context->argument(1);
                    if(obj.isValid())
                    {
                        QScriptValue data = engine->toObject(obj);
                        if(data.isObject())
                        {
                            QScriptValueIterator it(data);
                            while (it.hasNext())
                            {
                                it.next();
                                QString searchId = it.name();
                                QDomElement tag = doc.elementById(searchId);
                                if(!tag.isNull())
                                {
                                    QDomNodeList chld = tag.childNodes();
                                    for(uint ch = 0;ch < chld.length();ch++){
                                        tag.removeChild(chld.at(ch));
                                    }
                                    if(it.value().isString()){
                                        QDomText text = doc.createTextNode(it.value().toString());
                                        tag.appendChild(text);
                                    }
                                    else if(it.value().isBool())
                                    {
                                        QDomNode paren = tag.parentNode();
                                        if(!it.value().toBool()){
                                          paren.removeChild(tag);
                                        }
                                    }
                                    else  if(it.value().isNumber())
                                    {
                                        qsreal val = it.value().toNumber();
                                        QDomText text = doc.createTextNode(QString::number(val));
                                        tag.appendChild(text);
                                    }else{
                                        //qDebug() << QString("Property %1 is '%2', not a string or bool").arg(it.name()).arg(QString(it.value().toVariant().typeName()));
                                    }
                                }else{
                                    qDebug() << QString("tag with id='%1' not found").arg(searchId);
                                }
                            }
                        }else{
                            qDebug() << "Not an object";
                        }
                    }else{
                        qDebug() << "Received object is invalid";
                    }
                    QString path = context->argument(0).toString();
                    int li = path.lastIndexOf(QLatin1String("."));
                    QString ext = path.right(path.length() - li);
                    QString saveFile = validatePath(path.left(li), engine) % "_tmp" % ext;
                    QRegExp ex("\n");
                    QString xml(doc.toByteArray());
                    xml.replace(ex,QString(""));
                    QString xml2 = xml.simplified();
                    xml2.replace(QString("> <"),QString("><"));

                    QString encoding = "UTF-8";
                    if(context->argumentCount()>2){
                        QScriptValue enc = context->argument(2);
                        if(enc.isValid()){
                            if(enc.isString()){
                                encoding = enc.toString();
                                qDebug() << QString("Will save xml file with %1 encoding").arg(encoding);
                            }
                        }
                    }

                    bool ok = writeFile(saveFile,xml2,encoding);
                    if(ok){
                        qDebug() << QString("Saved %1").arg(saveFile);
                        return engine->toScriptValue(true);
                    }

                }else{
                    qDebug() << "Failed to load XML file";
                }
            }else{
                qDebug() <<QString("applyTemplate: wrong number of arguments %1. Required 2").arg(context->argumentCount());
            }
            return engine->toScriptValue(false);
        }
/*---------------------------------------------------------------------------------------*/

    QScriptValue listDir(QScriptContext *context, QScriptEngine *engine)
    {
        QVariantMap dirs;
        QString path =   validatePath(context->argument(0).toString(),engine);
        bool filesOnly = context->argument(1).toBool();
        QString _fltr =  context->argument(2).toString();
        QString fltr =   _fltr!="undefined"?_fltr:"*";

        QDir dir(path,fltr,QDir::Unsorted);
        if(filesOnly){
            dir.setFilter(FilesOnly);
        }else{
            dir.setFilter(DirsOnly);
        }
        if(dir.exists()){
            QFileInfoList fi = dir.entryInfoList();
            foreach(QFileInfo f, fi){
                dirs.insert(f.fileName(),f.absoluteFilePath());
            }
            return engine->toScriptValue(dirs);
        }else{
            return context->throwError(QString("Dir does not exists: %1").arg(path));
        }
    }

/*---------------------------------------------------------------------------------------*/
    void cleanupDir(const QString &dirPath,
                    const QString &extFilter,
                    QStringList fileList)
    {
        QDir cont(dirPath,extFilter,QDir::Unsorted,AllDirsAndFiles);

        QFileInfoList files = cont.entryInfoList();

        foreach (QFileInfo fileInfo, files){
            QString fname = fileInfo.fileName();
            if(fileList.contains(fname,Qt::CaseInsensitive)){
                //мы нашли в папке файл, который указан в списке файлов
                qDebug() << "Ok. File is in list: " % fileInfo.absoluteFilePath();
            }else{
                bool ok;
                ok = QFile(fileInfo.absoluteFilePath()).remove();
                if(ok){
                    qDebug() << "Removed extra file " % fileInfo.absoluteFilePath();
                }else{
                    qDebug() << "Failed to remove extra file " % fileInfo.absoluteFilePath();
                }
            }
        }
    }
/*--------------------------------------------------------------------------------------*/
    bool removeDir(const QString &dirName)
    {
        bool result = true;
        QDir dir(dirName);

        if (dir.exists(dirName)){
            Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
                if(info.isDir()){
                    result = removeDir(info.absoluteFilePath());
                }
                else{
                    result = QFile::remove(info.absoluteFilePath());
                }
                if (!result) {
                    return result;
                }
            }
            result = dir.rmdir(dirName);
        }
        return result;
    }
/*----------------------------------------------------------------------------------------*/
    QScriptValue remDir(QScriptContext *context, QScriptEngine *engine)
    {
        bool dirmoved = false;
        QString dirpath = context->argument(0).isString()? context->argument(0).toString():QString();
        if(!dirpath.isEmpty()){
            dirmoved = removeDir(validatePath(dirpath,engine));
            if(dirmoved){
                qDebug() << QString("Dir %1 successfully removed").arg(dirpath);
            }else{
                qDebug() << QString("Failed to remove dir %1").arg(dirpath);
            }
        }
        return engine->toScriptValue(dirmoved);
    }
/*----------------------------------------------------------------------------------------*/
QString minimizeCss(const QString &body)
{
  QString ret = body;
  ret.replace(QRegExp("[a-zA-Z]+#"),   "#" );
  ret.replace(";}", "}");
  ret.replace(QRegExp("([s:]0)(px|pt|%|em)"),"\\1");
  ret.replace(QRegExp("/*[dD]*?*/"),"");
  ret.replace(QRegExp("\t")," ");
  ret.replace(QRegExp("\r"),"");
  ret.replace(QRegExp("\n"),"");
  return ret.simplified();
}
/*----------------------------------------------------------------------------------------*/
    void _copyFile(const QString &source,
                  const QString &destination)
    {
        QFile f(source);
        qDebug() << "Copy " % source % " to : " % destination;
        bool copied = false;
        QFile newFile(destination);
        if(newFile.exists()){
            copied = newFile.remove();
        }
        if(f.fileName().endsWith(".css",Qt::CaseInsensitive))
        {
            QString css  = _readTextFile(source);
            QString _css = minimizeCss(css);
            writeFile(destination,_css,"UTF-8");
        }
        else
        {
            copied = f.copy(destination);
        }
        if(!copied){
            qDebug() << "Error copyng " + destination;
        }
    }
/*----------------------------------------------------------------------------------------*/
    QString _dirPathFromFilePath(QString filePath){
        return QFileInfo(filePath).dir().absolutePath() + slh;
    }
/*----------------------------------------------------------------------------------------*/
    QScriptValue dirPathFromFilePath(QScriptContext *context, QScriptEngine *engine){
        QString source  = validatePath(context->argument(0).toString(),engine);
        QString dirpath =_dirPathFromFilePath(source);
        return engine->toScriptValue(dirpath);
    }
/*----------------------------------------------------------------------------------------*/
    QScriptValue copyFile(QScriptContext *context, QScriptEngine *engine){
        if(context->argumentCount() >= 2)
        {
            QString source =   validatePath(context->argument(0).toString(),engine);
            QString destin =   validatePath(context->argument(1).toString(),engine);
            QDir di;
            if(di.mkpath(_dirPathFromFilePath(destin))){
                _copyFile(source,destin);
            }else{
                qDebug() << QString("Failed to make dir for %1").arg(destin);
            }
        }
        return engine->toScriptValue(false);
    }
/*----------------------------------------------------------------------------------------*/
    void _copyToDir(const QString &srceDir, // откуда
                   const QString &destDir,  // куда
                   const QString &nFilter,  // расширения файлов, которые надо копировать
                   bool deleteSource,       // стереть исх. папку? Режим перемещения папки
                   const QString &excludeItemsContaining,
                   const QString &excludeItemsContainTwo) //копировать, исключая файлы или папки содержащие эту подстроку
    {
        QFileInfo dinf(srceDir);
        if(dinf.isDir() && dinf.exists())
        {
           if((excludeItemsContaining.length()> 0 && dinf.baseName().contains(excludeItemsContaining,Qt::CaseInsensitive)) ||
              (excludeItemsContainTwo.length()> 0 && dinf.baseName().contains(excludeItemsContainTwo,Qt::CaseInsensitive))){
              return;
           }

            QDir d(srceDir,"*",QDir::Unsorted,AllDirsAndFiles);

            if(d.count()>0)
            {
                qDebug() << "Make mkpath: " + destDir;
                d.mkpath(destDir);
                QFileInfoList files = d.entryInfoList();
                QStringList filters = nFilter.contains(",")?nFilter.split(","):QStringList() << nFilter;//Ждем строку типа '*.cpp,*.cxx,*.cc'
                d.setNameFilters(filters);
                foreach (QFileInfo fileInfo, files)
                {
                    if( excludeItemsContaining.length() == 0 ||
                       !fileInfo.baseName().contains(excludeItemsContaining,Qt::CaseInsensitive) ||
                       !fileInfo.baseName().contains(excludeItemsContainTwo,Qt::CaseInsensitive))
                    {
                        QString source = fileInfo.absoluteFilePath();
                        QString destination = QDir::cleanPath(destDir + slh + fileInfo.fileName());
                        if(fileInfo.isDir())
                        {
                            if(!fileInfo.baseName().contains('@')){
                                _copyToDir(source,
                                           destination,
                                           nFilter,false,
                                           excludeItemsContaining,
                                           excludeItemsContainTwo);
                            }else{
                                qDebug() << QString("Dir %1 containg '@' character, skipping").arg(fileInfo.baseName());
                            }

                        }else{
                            _copyFile(source,destination);
                        }
                    }
                }
                if(deleteSource){//Удалить старую папку
                    qDebug() << "Rem dir: " + srceDir;
                    bool okRem = removeDir(srceDir);
                    if(!okRem)
                        qDebug() << "Error removing dir " + srceDir;
                }

            }else{
                qDebug() << srceDir % " dir does not contains thumbs";
            }
        }else{
            qDebug() << srceDir + " dir does not exists or is not a dir";
        }
    }
    QScriptValue copyToDir(QScriptContext *context, QScriptEngine *engine)
    {
        if(context->argumentCount() >= 2){
            QString source = validatePath(context->argument(0).toString(),engine);
            QString destin = validatePath(context->argument(1).toString(),engine);
            QString filtrs = context->argumentCount()>2?context->argument(2).isString()?context->argument(2).toString():"":"";
            QString exclude = context->argumentCount()>3?context->argument(3).isString()?context->argument(3).toString():"":"";
            QString excludeTwo = context->argumentCount()>4?context->argument(4).isString()?context->argument(4).toString():"":"";
            _copyToDir(source,
                       destin,
                       filtrs,
                       false,
                       exclude,
                       excludeTwo);
        }
        return engine->toScriptValue(false);
    }
/*--------------------------------------------------------------------------------------*/
    void addSetting(const QString &param, const QVariant &value){
        settings.setValue(param, value);
    }
/*--------------------------------------------------------------------------------------*/
    QVariant getSetting(const QString &param, const QVariant &defval){
        return settings.value(param, defval);
    }
/*--------------------------------------------------------------------------------------*/
    void setGroupSetting(const QString &groupName,
                                     const QString &key,
                                     const QString &value)
    {
        QString clKey = cleanStr(key, false);
        QString _key = groupName % "/" % clKey;
        settings.setValue(_key,value);
    }
/*--------------------------------------------------------------------------------------*/
    void clearGroupSetting(const QString &groupName){
        settings.beginGroup(groupName);
        settings.remove("");
        settings.endGroup();
    }
/*--------------------------------------------------------------------------------------*/
    QVariantMap getGroupSetting(const QString &groupName)
    {
        QVariantMap ret;
        settings.beginGroup(groupName);
        QStringList keys = settings.childKeys();
        foreach(QString key, keys){
            QVariant val = settings.value(key);
            if(QString(val.typeName()) == "QString"){
                ret.insert(key,val.toString());
            }
        }
        settings.endGroup();
        return ret;
    }
/*--------------------------------------------------------------------------------------*/
    QScriptValue fileInfo(QScriptContext *context, QScriptEngine *engine)
    {
        QVariantMap file;
        //if(context->argumentCount() == 1)
        //{
            QString path = context->argument(0).toString();
            QString filepath = validatePath(path, engine);
            QFileInfo fi(filepath);
            if(fi.exists()){
                file.insert("exists",        QVariant::fromValue(true));
                file.insert("absoluteFilePath",QVariant::fromValue(fi.absoluteFilePath()));
                file.insert("completeSuffix",QVariant::fromValue(fi.completeSuffix()));
                file.insert("suffix",        QVariant::fromValue(fi.suffix()));
                file.insert("created",       QVariant::fromValue(fi.created()));
                file.insert("readable",      QVariant::fromValue(fi.isReadable()));
                file.insert("size",          QVariant::fromValue(fi.size()));
                file.insert("lastModified",  QVariant::fromValue(fi.lastModified()));
                file.insert("lastRead",      QVariant::fromValue(fi.lastRead()));
                if(fi.suffix() == "exe" || fi.suffix() == "dll"){
                    //file.insert("fileVersion",QVariant::fromValue(getFileVersion(filepath)));
                    file.insert("fileVersion","");
                }else{
                    file.insert("fileVersion","");
                }
            }else{
                file.insert("exists",false);
            }
        //}
        return engine->toScriptValue(file);
    }
/*--------------------------------------------------------------------------------------*/
    QScriptValue makePath(QScriptContext *context, QScriptEngine *engine)
    {
        Q_UNUSED(engine);
        if(context->argumentCount() == 1)
        {
            if(context->argument(0).isString() &&
               context->argument(0).isValid()){
                QString _filePath = cleanStr(context->argument(0).toString(),true,true);
                QString filePath = translit(_filePath.toLocal8Bit());
                return engine->toScriptValue(filePath);
            }else{
                return engine->toScriptValue(QString());
            }
        }else{
            return engine->toScriptValue(QString());
        }
    }
/*--------------------------------------------------------------------------------------*/
    QScriptValue save(QScriptContext *context, QScriptEngine *engine)
    {
        Q_UNUSED(engine);
        if(context->argumentCount() == 2)
        {
            if(context->argument(0).isString() &&
               context->argument(0).isValid() &&
               context->argument(1).isValid())
            {
                QString _filePath = cleanStr(context->argument(0).toString(),true,true);
                QString filePath = translit(_filePath.toLocal8Bit());
                QString fileData = "";
                if(context->argument(1).isString())
                {
                    fileData = context->argument(1).toString();
                }
                else if(context->argument(1).isObject())
                {
                    /*QJson::Serializer serializer;
                    serializer.allowSpecialNumbers(false);
                    serializer.setIndentMode(QJson::IndentFull);
                    QByteArray json = serializer.serialize(context->argument(1).toVariant());
                    fileData = QString(json);*/
                }
                QString path = validatePath(filePath, engine);
                //qDebug() << QString("Saving file to %1").arg(path);
                writeFile(path,fileData,"UTF-8");
            }else{
                qDebug() << QString("One of arguments is not a string as it should");
            }
        }else{
            qDebug() << QString("Wrong number of arguments: %1").arg(context->argumentCount());
        }
        return engine->toScriptValue(false);
    }
/*--------------------------------------------------------------------------------------*/
    QScriptValue runProcess(QScriptContext *context, QScriptEngine *engine){
        if(context->argumentCount() > 0)
        {
            QString filePath = context->argument(0).toString();
          //Рабочая папка
            /*QString currScriptPath = engine->globalObject().property("qs").property("script").property("absoluteFilePath").toString();
            QFileInfo workFile(currScriptPath);
            QDir workDir(workFile.absoluteFilePath());
            QString workDirPath = workDir.absolutePath() % slh;
            qDebug() << QString("Set working dir to %1").arg(workDirPath);*/

            qDebug() << QString("Process: %1").arg(filePath);

            QProcess runner;
            //runner.setWorkingDirectory(workDirPath);
            runner.setProcessChannelMode(QProcess::SeparateChannels);//QProcess::SeparateChannels //QProcess::MergedChannels
            runner.start(filePath,QIODevice::ReadWrite);
            if(!runner.waitForStarted())
                return engine->toScriptValue(false);

            QByteArray data;
            while(runner.waitForReadyRead())
                data.append(runner.readAllStandardOutput());//.readAll()

            qDebug() << data.data();

            runner.waitForFinished(360000);

            return engine->toScriptValue(true);

        }else{
            return engine->toScriptValue(false);
        }
    }
/*---------------------------------------------------------------------------------------*/
    QScriptValue resolvePath(QScriptContext *context, QScriptEngine *engine){
        if(context->argumentCount() > 0){
            QString filePath = context->argument(0).toString();
            return engine->toScriptValue(validatePath(filePath, engine));
        }else{
            return engine->toScriptValue(QString());
        }
    }

/*========================================================================================*/
/*
char *fullpath (const char *path)
{
    LPTSTR lpFilePart;
    DWORD nBufferLength = 0;
    LPTSTR lpBuffer = NULL;
    nBufferLength = GetFullPathName (path, 0, lpBuffer, &lpFilePart);
    if (!nBufferLength)
        return path;
    lpBuffer = (LPTSTR) malloc (nBufferLength + 1);
    if (!lpBuffer)
        return path;
    if (GetFullPathName (path, nBufferLength, lpBuffer, &lpFilePart))
        return lpBuffer;
    else {
        free (lpBuffer);
        return path;
    }
}
char *searchpath (const char *path, const char *ext)
{
    LPTSTR lpFilePart, lpExt = NULL;
    DWORD nBufferLength = 0;
    LPTSTR lpBuffer = NULL;
    if (!strchr (path, '.'))
        lpExt = ext;
    nBufferLength = SearchPath (NULL, path, lpExt, 0, lpBuffer, &lpFilePart);
    if (!nBufferLength)
        return path;
    lpBuffer = (LPTSTR) malloc (nBufferLength + 1);
    if (!lpBuffer)
        return path;
    if (SearchPath (NULL, path, lpExt, nBufferLength, lpBuffer, &lpFilePart))
        return lpBuffer;
    else {
        free (lpBuffer);
        return path;
    }
}*/
/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/
    /*QScriptValue getFileVersion(QScriptContext *context, QScriptEngine *engine){
        if(context->argumentCount() > 0)
        {
            QString filePath = "";
            if(context->argument(0).isString())
            {
                filePath = context->argument(0).toString();
                if(QFileInfo(filePath).exists()){
                    QString s = getFileInfo(filePath);
                    return engine->toScriptValue(s);
                }else{
                    return engine->toScriptValue(QString());
                }
            }
            return engine->toScriptValue(QString());
        }else{
            return engine->toScriptValue(QString());
        }
    }*/
}
