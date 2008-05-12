mkdir Frameworks
cp -R /Library/Frameworks/QtCore.framework Frameworks
cp -R /Library/Frameworks/QtGui.framework Frameworks/
cp -R /Library/Frameworks/QtXml.framework Frameworks
cp -R /Library/Frameworks/QtNetwork.framework Frameworks/
cp -R /Library/Frameworks/QtWebKit.framework Frameworks/
mkdir plugins
cd plugins
mkdir imageformats
cd ..
cp -R /Developer/Applications/Qt/plugins/imageformats/libqjpeg.dylib plugins/imageformats

install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore MacOS/merkaartor 
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4.0/QtGui MacOS/merkaartor 
install_name_tool -change QtNetwork.framework/Versions/4/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/4.0/QtNetwork MacOS/merkaartor 
install_name_tool -change QtXml.framework/Versions/4/QtXml @executable_path/../Frameworks/QtXml.framework/Versions/4.0/QtXml MacOS/merkaartor 
install_name_tool -change QtWebKit.framework/Versions/4/QtWebKit @executable_path/../Frameworks/QtWebKit.framework/Versions/4.0/QtWebKit MacOS/merkaartor 

install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore Frameworks/QtGui.framework/Versions/4.0/QtGui 
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore Frameworks/QtNetwork.framework/Versions/4.0/QtNetwork 
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore Frameworks/QtXml.framework/Versions/4.0/QtXml
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore Frameworks/QtWebKit.framework/Versions/4.0/QtWebKit
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore plugins/imageformats/libqjpeg.dylib

install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4.0/QtGui Frameworks/QtWebKit.framework/Versions/4.0/QtWebKit
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4.0/QtGui plugins/imageformats/libqjpeg.dylib

install_name_tool -change QtNetwork.framework/Versions/4/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/4.0/QtNetwork Frameworks/QtWebKit.framework/Versions/4.0/QtWebKit
