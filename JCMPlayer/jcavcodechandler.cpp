#include "jcavcodechandler.h"
#include <QDebug>
#include <QAudioDevice>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QTimer>


void JCAVCodecHandler::ChangeSlider()
{
    std::lock_guard<std::mutex>lk(my_AVRead_Mutex);

    int64_t value = slider-> value() * AV_TIME_BASE;

    m_videoPktQueue.Clear();
    m_audioPktQueue.Clear();

    int ret=-1;
    int64_t ts= av_rescale_q(value,AV_TIME_BASE_Q,*m_pAudioRational);
    ret= avformat_seek_file(m_pFormatCtx,m_audioStreamIdx,INT64_MIN,ts,INT64_MAX,0);

    qDebug()<<ret;

    avcodec_flush_buffers(m_pAudioCodecCtx);
    avcodec_flush_buffers(m_pVideoCodecCtx);

    m_eMediaPlayStaute=MEDIAPLAY_STATUS_PAUSE;
}

JCAVCodecHandler::JCAVCodecHandler(QObject *parent) : QObject(parent){
     avformat_network_init();
     m_pFormatCtx = avformat_alloc_context();
     my_OpenGL= new common::qt::VideoOpenGL;
     my_AudioDevices=new QMediaDevices;
}

JCAVCodecHandler::~JCAVCodecHandler()
{
    UnInitVideoCodec();
}

void JCAVCodecHandler::SetVideoFilePath(const QString &path)
{
    m_videoPathString = path;
}

QString JCAVCodecHandler::GetVideoFilePaht()
{

    return m_videoPathString;
}


int JCAVCodecHandler::InitVideoCodec()
{
    if(m_videoPathString.isEmpty())
    {
        qDebug()<<"Path is NULL";
        return -1;
    }
    if(avformat_open_input(&m_pFormatCtx,m_videoPathString.toStdString().c_str(),NULL,NULL)!=0)
    {
        qDebug()<<"open input stream error";
        return -1;
    }
    if(avformat_find_stream_info(m_pFormatCtx,NULL)<0)
    {
        qDebug()<<"avformat_find_stream_info error";
        return -1;
    }


    for(int i=0;i<(int) m_pFormatCtx->nb_streams;i++)
    {
        AVCodecParameters * codec=m_pFormatCtx->streams[i]->codecpar;

        if(codec->codec_type==AVMEDIA_TYPE_AUDIO) {
            m_audioStreamIdx=i;
            m_pAudioCodecCtx=avcodec_alloc_context3(NULL);

            avcodec_parameters_to_context(m_pAudioCodecCtx,m_pFormatCtx->streams[i]->codecpar);
            const AVCodec *codec=avcodec_find_decoder(m_pAudioCodecCtx->codec_id);
            avcodec_open2(m_pAudioCodecCtx,codec,NULL);

        }
        if(codec->codec_type==AVMEDIA_TYPE_VIDEO) {
            m_videoStreamIdx=i;
            m_pVideoCodecCtx=avcodec_alloc_context3(NULL);

            avcodec_parameters_to_context(m_pVideoCodecCtx,m_pFormatCtx->streams[i]->codecpar);
            const AVCodec *codec=avcodec_find_decoder(m_pVideoCodecCtx->codec_id);
            avcodec_open2(m_pVideoCodecCtx,codec,NULL);

        }
    }
    av_dump_format(m_pFormatCtx,NULL,m_videoPathString.toStdString().c_str(),NULL);

    m_videoWidth=m_pVideoCodecCtx->width;
    m_videoHeight=m_pVideoCodecCtx->height;

    my_OpenGL->set_image_size(m_videoWidth,m_videoHeight,common::qt::VideoOpenGL::IMAGE_FORMAT_RGB888);

    if(m_pYUVFrame==NULL) m_pYUVFrame=av_frame_alloc();

    if(RGB24Buffer==nullptr){

        RGB24BufferSize=av_image_get_buffer_size(AV_PIX_FMT_RGB24,
                                                     m_videoWidth,m_videoHeight,1);
        RGB24Buffer=(uint8_t*) malloc(RGB24BufferSize);

        av_image_fill_arrays(m_pYUVFrame->data,m_pYUVFrame->linesize,RGB24Buffer,
                             AV_PIX_FMT_RGB24, m_videoWidth,m_videoHeight,1);
    }

    m_pVideoRational = &m_pFormatCtx->streams[m_videoStreamIdx]->time_base;
    m_pAudioRational = &m_pFormatCtx->streams[m_audioStreamIdx]->time_base;


    switch(m_pAudioCodecCtx->sample_fmt){
    case AV_SAMPLE_FMT_U8:
        m_SampleSize=8;break;
    case AV_SAMPLE_FMT_S16:
        m_SampleSize=16;break;
    case AV_SAMPLE_FMT_S32:
        m_SampleSize=32;break;
    default:break;

    }
    my_AudioFormat.setSampleRate(m_pAudioCodecCtx->sample_rate);
    my_AudioFormat.setChannelCount(m_pAudioCodecCtx->channels);
    my_AudioFormat.setSampleFormat(QAudioFormat::Int16);

  //  my_AudioFormat.setByteOrder(QAudioFormat::LittleEndian);


    my_AudioDevice = my_AudioDevices->defaultAudioOutput();
    QAudioDevice info(QMediaDevices::defaultAudioOutput());

    if (!info.isFormatSupported(my_AudioFormat)) {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
    }


    my_AudioSink= new QAudioSink(my_AudioDevice,my_AudioFormat,this);

    qDebug()<<m_audioStreamIdx<<" A "<<m_videoStreamIdx<<" V ";
    qDebug()<<m_pVideoRational->den<<" "<<m_pVideoRational->num;
    qDebug()<<m_pAudioRational->den<<" "<<m_pAudioRational->num;

    WidgetView = new QWidget;
    WidgetView ->setMinimumHeight(600);
    WidgetView ->setMinimumWidth(800);
    QPushButton *start=new QPushButton("启动");
    QPushButton *pause=new QPushButton("暂停");

    slider = new QSlider(Qt::Horizontal);
    slider->setRange(0,m_pFormatCtx->duration/1000000);

    connect(start,SIGNAL(clicked(bool)),this,SLOT(StartVideoSlot()));
    connect(pause,SIGNAL(clicked(bool)),this,SLOT(PauseVideoSlot()));
    connect(this,SIGNAL(closeEvent()),this,SLOT(CloseWindows()));
    connect(slider, &QSlider::sliderReleased, this,&JCAVCodecHandler::ChangeSlider);
    connect(slider, &QSlider::sliderPressed,[&](){
        m_eMediaPlayStaute=MEDIAPLAY_STATUS_SEEK;
    });
    QHBoxLayout *HL = new QHBoxLayout;

    HL->addWidget(start);
    HL->addWidget(pause);

    QVBoxLayout *VL = new QVBoxLayout;
    VL->addWidget(my_OpenGL);
    VL->addWidget(slider);
    VL->addLayout(HL);

    WidgetView->setLayout(VL);

    return 0;
}

void JCAVCodecHandler::UnInitVideoCodec()
{
    m_bThreadRuning = 0;
    for(int i=0;i<3;i++)
    {
        if(m_pworkThread[i]!=nullptr)
        {
            m_pworkThread[i]->join();
            m_pworkThread[i]=nullptr;
        }
    }
    delete WidgetView;
    WidgetView = nullptr;
}

void JCAVCodecHandler::StartVideo()
{
    startMedioProcessThread();
}

void JCAVCodecHandler::StopVideo()
{

}

void JCAVCodecHandler::startMedioProcessThread()
{
    m_bThreadRuning = 1;
    WidgetView->show();

    m_pworkThread[0] = new std::thread(&JCAVCodecHandler::doReadMideoPacketThread,this);
    m_pworkThread[1] = new std::thread(&JCAVCodecHandler::doVedioDecodeThread,this);
    m_pworkThread[2] = new std::thread(&JCAVCodecHandler::doAudioDecodeThread,this);

    my_AudioBuffer.open(QIODevice::ReadWrite);
    my_AudioSink->start(&my_AudioBuffer);
    my_clock.initClock();
}

void JCAVCodecHandler::convertAndRenderVideo(AVFrame *decodeFrame, long long pps)
{
    if(decodeFrame==NULL) return;

    if(m_pVideoSWrCtx==NULL)
    {
        m_pVideoSWrCtx = sws_getContext(m_videoWidth,m_videoHeight,
                                        m_pVideoCodecCtx->pix_fmt,
                                        m_videoWidth,m_videoHeight,
                                        AV_PIX_FMT_RGB24,SWS_BICUBIC
                                        ,NULL,NULL,NULL);
    }
    sws_scale(m_pVideoSWrCtx,decodeFrame->data,decodeFrame->linesize,0
              ,m_pVideoCodecCtx->height,m_pYUVFrame->data,m_pYUVFrame->linesize);
    double pts = my_clock.GetBack();

    double sleep_time = ((double)pps/m_pVideoRational->den-pts/m_pAudioRational->den)*1000;

    if(sleep_time < 0) return;

    stdThreadSleep(sleep_time);

    my_OpenGL->update_image((char*)RGB24Buffer);
}
void JCAVCodecHandler::convertAndAudioOut(AVFrame *decodeFrame,long long pps)
{

    if(!decodeFrame)
    {
        return;
    }
    if(!m_pAudioSWrCtx)
    {
         m_pAudioSWrCtx=swr_alloc_set_opts(NULL,
                                            m_pAudioCodecCtx->channel_layout,AV_SAMPLE_FMT_S16,m_pAudioCodecCtx->sample_rate
                                            ,m_pAudioCodecCtx->channel_layout,m_pAudioCodecCtx->sample_fmt,m_pAudioCodecCtx->sample_rate
                                            ,0,NULL);
        if(!m_pAudioSWrCtx) return;
        int len=swr_init(m_pAudioSWrCtx);
        if(len<0){
             qDebug()<<"error SWrctx len:"<<len;
             return;
        }
    }

    uint32_t dataSize = av_samples_get_buffer_size(NULL, decodeFrame->channels, decodeFrame->nb_samples,
                                              AV_SAMPLE_FMT_S16, 0);

    if(dataSize > my_audioBufferSize)
    {
        if(my_audioBuffer!=NULL)av_free(my_audioBuffer);
        my_audioBufferSize=dataSize;
        av_samples_alloc(&my_audioBuffer,decodeFrame->linesize, decodeFrame->channels,
             decodeFrame->nb_samples,AV_SAMPLE_FMT_S16,0);
    }
    swr_convert(m_pAudioSWrCtx,&my_audioBuffer,decodeFrame->nb_samples
                ,(const uint8_t**)decodeFrame->data,decodeFrame->nb_samples);

    my_AudioBuffer.open(QIODevice::ReadWrite);
    my_AudioBuffer.write((const char*)my_audioBuffer,dataSize);
    my_AudioBuffer.open(QIODevice::ReadOnly);

    double count = my_clock.setClock(pps);
   // double time_now = my_clock.setTime();
    double sleep_time = count*1000/m_pAudioRational->den;
    if(sleep_time>1000) sleep_time=1;
    stdThreadSleep(sleep_time-1);
    slider->setValue(pps/m_pAudioRational->den);
    //av_free(my_audioBuffer);
    // sum++;
    // qDebug()<<my_AudioBuffer.size()<<" "<<sum*2304;

}

void JCAVCodecHandler::doReadMideoPacketThread()
{

    while(m_bThreadRuning)
    {
        if(m_eMediaPlayStaute==MEDIAPLAY_STATUS_SEEK||m_audioPktQueue.size()>600||m_videoPktQueue.size()>1200){
            stdThreadSleep(10);
            continue;
        }
        readMediaPacket();
    }
    qDebug()<<"Packet codec thread exit";
}

void JCAVCodecHandler::doVedioDecodeThread()
{
    if(m_pFormatCtx==NULL) return;

    while(m_bThreadRuning)
    {
        if(WidgetView->isHidden())
        {
            emit closeEvent();
            break;
        }
        if(m_eMediaPlayStaute==MEDIAPLAY_STATUS_PAUSE||m_eMediaPlayStaute==MEDIAPLAY_STATUS_SEEK){
            stdThreadSleep(10);
            continue;
        }

        if(m_videoPktQueue.isEmpty())
        {
            stdThreadSleep(1);
            continue;
        }
        AVPacket *packet = m_videoPktQueue.dequeue();
        if(packet==NULL){
            continue;
        }
        int ret=avcodec_send_packet(m_pVideoCodecCtx,packet);
        if(ret!=0)
        {
            av_packet_free(&packet);
            free(packet);
            break;
        }
        while(true){
           m_pVideoFrame=av_frame_alloc();
           ret = avcodec_receive_frame(m_pVideoCodecCtx,m_pVideoFrame);
          if(ret==0)
          {
             convertAndRenderVideo(m_pVideoFrame,m_pVideoFrame->pts);
          }else{
            break;
          }
          av_frame_unref(m_pVideoFrame);
        }
        av_packet_free(&packet);
        free(packet);
    }
    qDebug()<<"vedio codec thread exit...";
}
void JCAVCodecHandler::doAudioDecodeThread()
{

    if(m_pFormatCtx==NULL) return;

    while(m_bThreadRuning)
    {
        if(m_eMediaPlayStaute==MEDIAPLAY_STATUS_PAUSE||m_eMediaPlayStaute==MEDIAPLAY_STATUS_SEEK){
            stdThreadSleep(10);
            continue;
        }

        if(m_audioPktQueue.isEmpty())
        {
            stdThreadSleep(1);
            continue;
        }
        AVPacket *packet = m_audioPktQueue.dequeue();
        if(packet==NULL){
            continue;
        }

        int ret=avcodec_send_packet(m_pAudioCodecCtx,packet);
        if(ret!=0)
        {
            av_packet_free(&packet);
            free(packet);
            break;
        }
        while(true){
            m_pAudioFrame=av_frame_alloc();
            ret = avcodec_receive_frame(m_pAudioCodecCtx,m_pAudioFrame);
            if(ret==0)
            {
                convertAndAudioOut(m_pAudioFrame,m_pAudioFrame->pts);
            }else{
                break;
            }
            av_frame_unref(m_pAudioFrame);
        }
        av_packet_free(&packet);
        free(packet);
    }
    qDebug()<<"Audio codec thread exit";
}

void JCAVCodecHandler::readMediaPacket()
{
    AVPacket *packet = av_packet_alloc();
    if(!packet){
        qDebug()<<"Error packet";
        return;
    }
    std::lock_guard<std::mutex>lk(my_AVRead_Mutex);
    int ret=av_read_frame(m_pFormatCtx,packet);

    if(ret==0){
        if(packet->stream_index==m_audioStreamIdx)
        {
            m_audioPktQueue.enqueue(packet);

        }else if(packet->stream_index==m_videoStreamIdx){
            m_videoPktQueue.enqueue(packet);

        }else if(packet!=NULL){
            av_packet_free(&packet);
            free(packet);
        }
    }else{
        if(m_eMediaPlayStaute==MEDIAPLAY_STATUS_PAUSE)
        {
            return;
        }
        if(ret==AVERROR_EOF){
            m_bReadFileEOF = 1;
        }
    }
}
void JCAVCodecHandler::stdThreadSleep(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

QSize JCAVCodecHandler::GetMediaWidthHeight()
{
    return QSize(m_videoWidth,m_videoHeight);
}
