Windows:
	Deployment:
		~:\Qt\6.`*.*`\mingw_64\bin\windeployqt6.exe Qalam.exe
	Packaging:
		~:\Qt\Tools\QtInstallerFramework\4.`*`\bin\binarycreator.exe -c config/config.xml -p packages QalamInstaller-Win-X64

Linux:
 Important:"use linux ubuntu version 22 only"
 
 sudo apt install libxcb-cursor0 libxcb-cusor-dev
 
 sudo apt install build-essential // for build on QtCreator
 sudo apt install qt6-base-dev // for build on QtCreator
 sudo apt install cmake // for build on QtCreator

 add path to env path:
  path form: export PATH=/path/to/qt/bin:$PATH
  
  nano ~/.bashrc
  write the path at the end of file then save and close
  source ~/.bashrc

 linuxdeployqt:
  sudo apt install libfuse2
  download linuxdeployqt-continuous-x86_64.AppImage from https://github.com/probonopd/linuxdeployqt
  chmod a+x linuxdeployqt-continuous-x86_64.AppImage
  ./linuxdeployqt-continuous-x86_64.AppImage AppNameHere -always-overwrite
  
  note: if qmake not exist add it to paths or use: -qmake=/home/name/Qt/6.*.*/gcc_64/bin/qmake
  note: for ubuntu version above 22 use this options => -unsupported-bundle-everything -unsupported-allow-new-glibc
  


MacOS:




Common:
 لتحميل مكتبة Qt6
 URL for HTTP download mirrors "https://download.qt.io/static/mirrorlist/"
 on cmd pass "NameOfQtOnlineInstaller.exe --mirror https://mirrors.ocf.berkeley.edu/qt/"
