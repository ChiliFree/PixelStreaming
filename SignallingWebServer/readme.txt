"AutoPreloadProcess": 预加载程序
"AutoDestroyProcessTime": 不立即关闭程序，主要应对刷新页面
"ProcessLogToCmd": UE的日志是否打印到控制台
"UnrealProcessPath": exe文件的路劲,
"UnrealProcessArgument": exe的命令行参数


"-PSForceH265",
//强制使用H265，想要8K只能使用H265,其他的建议H264，因为Chrome webrtc还不支持H265，H265需要定制的浏览器
"-AudioMixer",
"-PixelStreamingIP=127.0.0.1",
"-ForceRes",
"-RenderOffScreen",
//离屏渲染，后台渲染，看不到窗口，使渲染可以超过显示器分辨率
"-Unattended",
"-ResX=1920",
"-ResY=1080",
"-PixelStreamingEncoderRateControl=VBR",
//动态码率
"-PixelStreamingEncoderMinQP=17",
"-PixelStreamingEncoderMinQP=50",
//Min与Max越小画质越好，但是更耗费GPU，也会影响码率，画质好，码率肯定高
"-PixelStreamingDegradationPreference=BALANCED",
//当码率太高或者网络差时，Webrtc优先保帧率还是优先保画质，默认即可
"-PixelStreamingEncoderTargetBitrate=50000000",
//编码器（NVENC）编码的目标码率，只是个参考值，可能会大于这个值，也可能会小于这个值，最终编码出来的码率，会影响到WebRTC是否主动丢帧
"-PixelStreamingEncoderMaxBitrate=500000000",
//编码器（NVENC）编码的最高码率
"-PixelStreamingWebRTCStartBitrate=200000000",
//WebRTC起使的码率，这个码率要介于Min与Max之间，是个参考值，实际码率会动态调整
"-PixelStreamingWebRTCMinBitrate=200000000",
//WebRTC最低码率，网络条件不好时，尽量保证码率不低于这个值，码率太低，会导致webrtc主动丢帧，造成视频卡顿，以及延时增加
"-PixelStreamingWebRTCMaxBitrate=500000000",
//WebRTC最高码率，码率上限
"-PixelStreamingWebRTCDisableReceiveAudio=1",
//不要音频，降低码率
"-PixelStreamingWebRTCDisableAudioSync=1",
//不要音视频同步，降低延时
"-PixelStreamingWebRTCDisableTransmitAudio=1",
//不需要传递音频给浏览器，降低码率
"-PixelStreamingEncoderMultipass=DISABLED",
//MultiPass多次遍历视频源，对编码优化，带来更好的画质，更低的码率，但是耗费GPU，默认可以DISABLE或QUARTER
"-PixelStreamingWebRTCMaxFps=60"
//UE渲染帧率与视频的最大帧率，帧率高码率高