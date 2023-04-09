export interface IAppState {
  servers: IServer[];
  files: IFile[];
}

export interface IServer {
  // int id;
  // QString sfdl;         // sfdl file
  // QString name;         // download name
  // QString path;         // base path on ftp server
  // QString ip;           // ftp server ip
  // qint16 port;          // ftp server port
  // QString user;         // ftp server username
  // QString pass;         // ftp server password
  // qint64 progress;      // overall download progress
  // qint64 total;         // total download size
  // int status;           // download status
  // qint64 dateStart;     // timestamp download start
  // qint64 dateStop;      // timestamp download stop
  // qint64 dateCancel;    // timestamp download canceled
  id: string;
  name: string;
  total: number;
}

export interface IFile {
  // int id;
  // int dServerID;        // dServer id
  // QString fileName;     // name of the file
  // QString fullFilePath; // full download path on ftp server
  // QString cleanPath;    // clean path without filename
  // QString subDirs;      // sub dir structur
  // qint64 progress;      // current download progress
  // qint64 total;         // total file size
  // int status;           // download status
  // qint64 dateStart;     // timestamp download start
  // qint64 dateStop;      // timestamp download stop
  // qint64 dateCancel;    // timestamp download canceled
  id: string;
  dServerID: string;
  fileName: string;
  total: number;
  progress: number;
}
