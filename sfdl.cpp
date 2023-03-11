#include "sfdl.h"

#include <QDebug>

#include <QCryptographicHash>
#include <qaesencryption.h>

sfdl::sfdl(QObject *parent) : QObject(parent)
{

}

QByteArray sfdl::loadSFDL(QString file)
{
    qDebug() << "loadSFDL: " << file;

    QFile f(file);

    if(f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray arr = f.readAll();
        f.close();

        return arr;
    }

    return QByteArray();
}

QString sfdl::decryptString(QString password, QString encodedString)
{
    QByteArray data = QByteArray::fromBase64(encodedString.toLatin1());
    QString iv = encodedString.section("", 0, 16);

    QAESEncryption encryption(QAESEncryption::AES_128, QAESEncryption::CBC, QAESEncryption::PADDING::PKCS7);

    QByteArray hashKey = QCryptographicHash::hash(password.toLocal8Bit(), QCryptographicHash::Md5);
    QByteArray hashIV = QCryptographicHash::hash(iv.toLocal8Bit(), QCryptographicHash::Md5);

    QByteArray decodeText = encryption.decode(data, hashKey, hashIV);
    QString decodedString = QString(encryption.removePadding(decodeText).mid(16));

    return decodedString;
}

bool sfdl::validateIPv4(QString ip)
{
    QHostAddress address(ip);

    if(QAbstractSocket::IPv4Protocol == address.protocol())
    {
       return true;
    }

    return false;
}

QString sfdl::ListElements(QDomElement root, QString tagname)
{
    QDomNodeList items = root.elementsByTagName(tagname);

    QString element;
    for(int i = 0; i < items.count(); i++)
    {
        QDomNode itemnode = items.at(i);
        if(itemnode.isElement())
        {
            QDomElement itemele = itemnode.toElement();
            element = itemele.text();
        }
    }
    return element;
}

void sfdl::setSFDL(QString file, QStringList passwordList)
{
    _SFDLFile = file;
    _passwordList = passwordList;
}

// void sfdl::readSFDL(QString file)
void sfdl::readSFDL()
{
    QString file = _SFDLFile;
    QStringList passwords = _passwordList;

    QByteArray data = loadSFDL(file);
    if(data.isEmpty())
    {
        // sendLogText("<font color=\"red\">" + file + tr(": Kann SFDL Datei nicht laden! (Keine Daten)</font>"));
        // sendWarning(tr("SFDL Error"), tr("Kann SFDL Datei nicht laden! (Keine Daten)") + "\n[" + file + "]");

        qDebug() << "Kann SFDL Datei nicht laden! (Keine Daten): " << file;

        return;
    }

    QDomDocument sfdl;

    if(!sfdl.setContent(data))
    {
        // sendLogText("<font color=\"red\">" + file + tr(": Kann SFDL Datei nicht laden! (Keine XML-Daten/Datei)</font>"));
        // sendWarning(tr("SFDL Error"), tr("Kann SFDL Datei nicht laden! (Keine XML-Daten/Datei)") + "\n[" + file + "]");

        qDebug() << "Kann SFDL Datei nicht laden! (Keine XML-Daten/Datei): " << file;

        return;
    }

    QStringList n_data;
    QStringList n_files;

    // sfdl file
    n_data.append("SFDLFile|" + file);

    QDomElement root = sfdl.firstChildElement();

    QString Description;
    QString Uploader;
    QString SFDLFileVersion;
    QString Encrypted;
    QString MaxDownloadThreads;

    Description = ListElements(root, "Description");
    Uploader = ListElements(root, "Uploader");
    SFDLFileVersion = ListElements(root, "SFDLFileVersion");
    Encrypted = ListElements(root, "Encrypted");
    MaxDownloadThreads = ListElements(root, "MaxDownloadThreads");


    qDebug() << "Description: " << Description;
    qDebug() << "Uploader: " << Uploader;
    qDebug() << "SFDLFileVersion: " << SFDLFileVersion;
    qDebug() << "Encrypted: " << Encrypted;
    qDebug() << "MaxDownloadThreads: " << MaxDownloadThreads;


    n_data.append("Description|" + Description);
    n_data.append("Uploader|" + Uploader);
    n_data.append("SFDLFileVersion|" + SFDLFileVersion);
    n_data.append("Encrypted|" + Encrypted);
    n_data.append("MaxDownloadThreads|" + MaxDownloadThreads);

    // ConnectionInfo
    QDomNodeList ConnectionInfos = root.elementsByTagName("ConnectionInfo");

    QString Name;

    QString Host;
    int Port;
    QString Username;
    QString Password;
    QString AuthRequired;

    QString DataConnectionType;
    QString DataType;
    QString CharacterEncoding;
    QString EncryptionMode;
    QString ListMethod;
    QString DefaultPath;
    QString ForceSingleConnection;
    QString DataStaleDetection;
    QString SpecialServerMode;

    for(int i = 0; i < ConnectionInfos.count(); i++)
    {
        QDomNode snode = ConnectionInfos.at(i);
        if(snode.isElement())
        {
            QDomElement info = snode.toElement();

            Name = ListElements(info, "Name");
            Host = ListElements(info, "Host");
            Port = ListElements(info, "Port").toInt();
            Username = ListElements(info, "Username");
            Password = ListElements(info, "Password");
            AuthRequired = ListElements(info, "AuthRequired");

            DataConnectionType = ListElements(info, "DataConnectionType");
            DataType = ListElements(info, "DataType");
            CharacterEncoding = ListElements(info, "CharacterEncoding");
            EncryptionMode = ListElements(info, "EncryptionMode");
            ListMethod = ListElements(info, "ListMethod");
            DefaultPath = ListElements(info, "DefaultPath");
            ForceSingleConnection = ListElements(info, "ForceSingleConnection");
            DataStaleDetection = ListElements(info, "DataStaleDetection");
            SpecialServerMode = ListElements(info, "SpecialServerMode");
        }
    }


    qDebug() << "Name: " << Name;
    qDebug() << "Host: " << Host;
    qDebug() << "Port: " << Port;
    qDebug() << "Username: " << Username;
    qDebug() << "Password: " << Password;
    qDebug() << "AuthRequired: " << AuthRequired;

    qDebug() << "DataConnectionType: " << DataConnectionType;
    qDebug() << "DataType: " << DataType;
    qDebug() << "CharacterEncoding: " << CharacterEncoding;
    qDebug() << "EncryptionMode: " << EncryptionMode;
    qDebug() << "ListMethod: " << ListMethod;
    qDebug() << "DefaultPath: " << DefaultPath;
    qDebug() << "ForceSingleConnection: " << ForceSingleConnection;
    qDebug() << "DataStaleDetection: " << DataStaleDetection;
    qDebug() << "SpecialServerMode: " << SpecialServerMode;



    if(Description.isEmpty() || Host.isEmpty() || !Port)
    {
        // sendLogText("<font color=\"red\">" + file + tr(": Unzureichende Daten in SFDL Datei! Kein Download möglich.</font>"));
        qDebug() << "Unzureichende Daten in SFDL Datei! Kein Download möglich: " << file;
        return;
    }

    n_data.append("Name|" + Name);
    n_data.append("Host|" + Host);
    n_data.append("Port|" + QString::number(Port));
    n_data.append("Username|" + Username);
    n_data.append("Password|" + Password);
    n_data.append("AuthRequired|" + AuthRequired);

    n_data.append("DataConnectionType|" + DataConnectionType);
    n_data.append("DataType|" + DataType);
    n_data.append("CharacterEncoding|" + CharacterEncoding);
    n_data.append("EncryptionMode|" + EncryptionMode);
    n_data.append("ListMethod|" + ListMethod);
    n_data.append("DefaultPath|" + DefaultPath);
    n_data.append("ForceSingleConnection|" + ForceSingleConnection);
    n_data.append("DataStaleDetection|" + DataStaleDetection);
    n_data.append("SpecialServerMode|" + SpecialServerMode);

    // check for password protection and decrypt sfdl data
    QString sfdlPassword = "mlcboard.com";
    // QString sfdlPassword = QString();

    if(n_data.at(4).split("|").at(1) == "true")
    {
        bool ipTextOK = false;

        /*
        for(int i = 0; i < passwords.count(); i++)
        {
            QString passTest = decryptString(passwords.at(i), n_data.at(7).split("|").at(1));

            if(passTest != "__fail__to__decrypt__" && !passTest.isEmpty() && validateIPv4(passTest))
            {
                sfdlPassword = passwords.at(i);
                ipTextOK = true;
                break;
            }
        }
        */

        QString passTest = decryptString(sfdlPassword, n_data.at(7).split("|").at(1));
        if(passTest != "__fail__to__decrypt__" && !passTest.isEmpty() && validateIPv4(passTest))
        {
            ipTextOK = true;
        }

        if(!ipTextOK)
        {
            // sendLogText("<font color=\"red\">" + file + tr(": Kann SFDL Datei nicht entschlüsseln! (Falsches Passwort: ") + sfdlPassword + ")</font>");
            // sendWarning(tr("SFDL Error"), tr("Kann SFDL Datei nicht entschlüsseln! (Falsches Passwort)") + "\n[" + file + "]");

            qDebug() << "Kann SFDL Datei nicht entschlüsseln! (Falsches Passwort: " + sfdlPassword + "): " << file;

            return;
        }

        // sendLogText(file + tr(": Erfogreich mit Passwort <b>") + sfdlPassword + tr("</b> entschlüsselt."));
        qDebug() << "Erfogreich mit Passwort " + sfdlPassword + " entschlüsselt." << file;

        for(int i = 0; i < n_data.count(); i++)
        {
            QString dataInfo = n_data.at(i).split("|").at(0);
            QString dataContent;

            if(dataInfo == "Host" ||
                    dataInfo == "Username" ||
                    dataInfo == "Password" ||
                    dataInfo == "Uploader" ||
                    dataInfo == "Description" ||
                    dataInfo == "DefaultPath")
            {
                QString dec = decryptString(sfdlPassword, n_data.at(i).split("|").at(1));

                if(n_data.at(i).split("|").at(0) == "Host")
                {
                    Host = dec;
                }

                if(n_data.at(i).split("|").at(0) == "Port")
                {
                    Port = dec.toInt();
                }

                if(n_data.at(i).split("|").at(0) == "Username")
                {
                    Username = dec;
                }

                if(n_data.at(i).split("|").at(0) == "Password")
                {
                    Password = dec;
                }

                dataContent = n_data.at(i).split("|").at(0) + "|" + dec;
                n_data[i] = dataContent;
            }
        }
    }

    QDomNodeList Packages = root.elementsByTagName("Packages");
    for(int i = 0; i < Packages.count(); i++)
    {
        QDomNode snode = Packages.at(i);
        if(snode.isElement())
        {
            QDomElement info = snode.toElement();
            if(ListElements(info, "BulkFolderMode") == "true")
            {
                QDomNodeList SFDLPackage = info.elementsByTagName("SFDLPackage");
                for(int i = 0; i < SFDLPackage.count(); i++)
                {
                    QDomNode snode = SFDLPackage.at(i);
                    if(snode.isElement())
                    {
                        QDomElement info = snode.toElement();

                        QDomNodeList FileList = info.elementsByTagName("BulkFolderList");
                        for(int i = 0; i < FileList.count(); i++)
                        {
                            QDomNode snode = FileList.at(i);
                            if(snode.isElement())
                            {
                                QDomElement info = snode.toElement();

                                QDomNodeList FileInfo = info.elementsByTagName("BulkFolder");
                                for(int i = 0; i < FileInfo.count(); i++)
                                {
                                    QDomNode snode = FileInfo.at(i);
                                    if(snode.isElement())
                                    {
                                        QDomElement info = snode.toElement();

                                        // qDebug() << "BulkFolderPath: " << ListElements(info, "BulkFolderPath");

                                        QString BulkFolderPath = ListElements(info, "BulkFolderPath");

                                        if(n_data.at(4).split("|").at(1) == "true")
                                        {
                                            BulkFolderPath = decryptString(sfdlPassword, BulkFolderPath);
                                        }

                                        n_data.append("BulkFolderPath|" + BulkFolderPath);

                                        #ifdef QT_DEBUG
                                            qDebug() << "BulkFolderPath (blank): " << BulkFolderPath;
                                        #endif
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                //qDebug() << "BulkFolderMode = false";
                n_data.append("BulkFolderPath|false");

                QDomNodeList SFDLPackage = info.elementsByTagName("SFDLPackage");
                for(int i = 0; i < SFDLPackage.count(); i++)
                {
                    QDomNode snode = SFDLPackage.at(i);
                    if(snode.isElement())
                    {
                        QDomElement info = snode.toElement();
                        //qDebug() << "Packagename: " << ListElements(info, "Packagename");

                        QDomNodeList FileList = info.elementsByTagName("FileList");
                        for(int i = 0; i < FileList.count(); i++)
                        {
                            QDomNode snode = FileList.at(i);
                            if(snode.isElement())
                            {
                                QDomElement info = snode.toElement();

                                QDomNodeList FileInfo = info.elementsByTagName("FileInfo");
                                for(int i = 0; i < FileInfo.count(); i++)
                                {
                                    QDomNode snode = FileInfo.at(i);
                                    if(snode.isElement())
                                    {
                                        QDomElement info = snode.toElement();

                                        qlonglong size = ListElements(info, "FileSize").toInt();

                                        QString theFilePath;
                                        theFilePath = ListElements(info, "DirectoryPath");

                                        QString FileFullPath;
                                        FileFullPath = ListElements(info, "FileFullPath");

                                        QString theFileName;
                                        theFileName = ListElements(info, "FileName");

                                        #ifdef QT_DEBUG
                                            qDebug() << "size: " << size;
                                            qDebug() << "theFilePath: " << theFilePath;
                                            qDebug() << "theFileName: " << theFileName;
                                            qDebug() << "FileFullPath: " << FileFullPath;
                                        #endif

                                        n_files.append(FileFullPath + "|" + QString::number(size));

                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // qDebug() << "n_data: " << n_data;
    // qDebug() << "n_files: " << n_files;

    emit sendSFDLData(n_data, n_files);
}
