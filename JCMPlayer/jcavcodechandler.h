#ifndef JCAVCODECHANDLER_H
#define JCAVCODECHANDLER_H

#include <QObject>
#include <QSize>
#include <thread>
#include <iostream>
#include <mutex>
#include <atomic>
#include <vector>
#include <jcdatedefine.h>
#include "videoopengl.h"
#include <QAudioFormat>
#include <QAudioSink>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QBuffer>
#include <QAudioBuffer>
#include <QFile>
#include <QSlider>
#include "avsync.h"


extern "C"
{

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>


}

enum MediaPlayStatus{
    MEDIAPLAY_STATUS_PLAYING=0,
    MEDIAPLAY_STATUS_PAUSE,
    MEDIAPLAY_STATUS_SEEK,
    MEDIAPLAY_STATUS_STOP
};
class JCAVCodecHandler:public QObject
{
    Q_OBJECT
private slots:
    void StartVideoSlot(){
        m_eMediaPlayStaute=MEDIAPLAY_STATUS_PLAYING;
    };
    void PauseVideoSlot(){
        m_eMediaPlayStaute=MEDIAPLAY_STATUS_PAUSE;
    };
    void CloseWindows(){
        UnInitVideoCodec();
    }
    void ChangeSlider();
 signals:
    void closeEvent();
public:
    explicit JCAVCodecHandler(QObject *parent = nullptr);
    virtual ~JCAVCodecHandler();

    void      SetVideoFilePath(const QString& path);
    QString   GetVideoFilePaht();

    QSize     GetMediaWidthHeight();

    int       InitVideoCodec();
    void      UnInitVideoCodec();
    void      startMedioProcessThread();
private:
    void      convertAndRenderVideo(AVFrame* decodeFrame,long long pps);
    void      convertAndAudioOut(AVFrame* decodeFrame,long long pps);

    void      StartVideo();
    void      StopVideo();

    void      doReadMideoPacketThread();
    void      doVedioDecodeThread();
    void      doAudioDecodeThread();

    void      readMediaPacket();
    void      stdThreadSleep(int ms);


private:
    QString  m_videoPathString = "";

    int      m_videoWidth      = 0;
    int      m_videoHeight     = 0;

    int      m_videoStreamIdx  = -1;
    int      m_audioStreamIdx  = -1;

    AVFormatContext* m_pFormatCtx      = nullptr;
    AVCodecContext * m_pVideoCodecCtx  = nullptr;
    AVCodecContext * m_pAudioCodecCtx  = nullptr;

    SwsContext     * m_pVideoSWrCtx    = nullptr;
    SwrContext     * m_pAudioSWrCtx    = nullptr;

    AVFrame *        m_pVideoFrame     = nullptr;
    AVFrame *        m_pAudioFrame     = nullptr;
    AVFrame *        m_pYUVFrame     = nullptr;

    AVRational     * m_pVideoRational  = nullptr;
    AVRational     * m_pAudioRational  = nullptr;

    std::thread *m_pworkThread[3];
    JCDateDefine<AVPacket*> m_videoPktQueue,m_audioPktQueue;

    uint8_t* RGB24Buffer=nullptr;
    uint32_t RGB24BufferSize=0;

    QAudioFormat my_AudioFormat;
    QAudioSink *my_AudioSink = nullptr;
    QMediaDevices *my_AudioDevices= nullptr;
    QAudioDevice my_AudioDevice;

   // QAudioBuffer my_AudioBuffer;
    QBuffer my_AudioBuffer;
    QByteArray audioData;

    uint8_t *my_audioBuffer=nullptr;
    uint32_t my_audioBufferSize=0;

    bool m_bReadFileEOF = false;
    int m_bThreadRuning = 0;

    int m_SampleRate=44100;
    int m_SampleSize=16;
    int m_Channels=2;
    int m_ValueRatio=1;

    QFile *File;
    std::vector<int> a;
    common::qt::VideoOpenGL *my_OpenGL=nullptr;

    std::atomic<MediaPlayStatus> m_eMediaPlayStaute=MEDIAPLAY_STATUS_PAUSE;

    AVSync my_clock;

    QWidget *WidgetView=nullptr;
    QSlider *slider = nullptr;

    std::mutex my_AVRead_Mutex;

};

#endif // JCAVCODECHANDLER_H
