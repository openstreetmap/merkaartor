#ifndef MYMESSAGEHANDLER_H
#define MYMESSAGEHANDLER_H

#if defined(Q_OS_WIN)
extern Q_CORE_EXPORT void qWinMsgHandler(QtMsgType t, const char* str);
#endif

#ifdef LOG_TO_FILE
#include <QFile>
#include <QTextStream>

QFile* pLogFile;
QTextStream ts;
#endif

void myMessageOutput(QtMsgType msgType, const char *buf)
{
// From corelib/global/qglobal.cpp : qt_message_output

//#if defined(Q_OS_WIN) && !defined(QT_NO_DEBUG_OUTPUT)
//    qWinMsgHandler(msgType, buf);
//#endif

#ifdef LOG_TO_FILE
    if (pLogFile) {
        ts << buf << endl;
    }
#endif

#if defined(Q_CC_MWERKS) && defined(Q_OS_MACX)
    mac_default_handler(buf);
#elif defined(Q_OS_WINCE)
    QString fstr = QString::fromLatin1(buf);
    fstr += QLatin1Char('\n');
    OutputDebugString(reinterpret_cast<const wchar_t *> (fstr.utf16()));
#elif defined(Q_OS_SYMBIAN)
    // RDebug::Print has a cap of 256 characters so break it up
    _LIT(format, "[Qt Message] %S");
    const int maxBlockSize = 256 - ((const TDesC &)format).Length();
    const TPtrC8 ptr(reinterpret_cast<const TUint8*>(buf));
    HBufC* hbuffer = q_check_ptr(HBufC::New(qMin(maxBlockSize, ptr.Length())));
    for (int i = 0; i < ptr.Length(); i += hbuffer->Length()) {
        hbuffer->Des().Copy(ptr.Mid(i, qMin(maxBlockSize, ptr.Length()-i)));
        RDebug::Print(format, hbuffer);
    }
    delete hbuffer;
#else
    fprintf(stderr, "%s\n", buf);
    fflush(stderr);
#endif

    if (msgType == QtFatalMsg
        || (msgType == QtWarningMsg
        && (!qgetenv("QT_FATAL_WARNINGS").isNull())) ) {

#if defined(Q_CC_MSVC) && defined(QT_DEBUG) && defined(_DEBUG) && defined(_CRT_ERROR)
    // get the current report mode
    int reportMode = _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);
    _CrtSetReportMode(_CRT_ERROR, reportMode);
#if !defined(Q_OS_WINCE)
    int ret = _CrtDbgReport(_CRT_ERROR, __FILE__, __LINE__, QT_VERSION_STR, buf);
#else
    int ret = _CrtDbgReportW(_CRT_ERROR, _CRT_WIDE(__FILE__),
        __LINE__, _CRT_WIDE(QT_VERSION_STR), reinterpret_cast<const wchar_t *> (QString::fromLatin1(buf).utf16()));
#endif
    if (ret == 0  && reportMode & _CRTDBG_MODE_WNDW)
        return; // ignore
    else if (ret == 1)
        _CrtDbgBreak();
#endif

#if defined(Q_OS_SYMBIAN)
    __DEBUGGER(); // on the emulator, get the debugger to kick in if there's one around
    TBuf<256> tmp;
    TPtrC8 ptr(reinterpret_cast<const TUint8*>(buf));
    TInt len = Min(tmp.MaxLength(), ptr.Length());
    tmp.Copy(ptr.Left(len));
    // Panic the current thread. We don't use real panic codes, so 0 has no special meaning.
    User::Panic(tmp, 0);
#elif (defined(Q_OS_UNIX) || defined(Q_CC_MINGW))
    abort(); // trap; generates core dump
#else
    exit(1); // goodbye cruel world
#endif
    }
}


#endif // MYMESSAGEHANDLER_H
