#ifndef AUDIOWIDGET_H
#define AUDIOWIDGET_H
#include <QAudioFormat>

class AudioWidget
{
public:
    AudioWidget();
private:
    QAudioFormat* my_AudioFormat;
};

#endif // AUDIOWIDGET_H
