# starrtc-iot-demo
集成文档：https://docs.starrtc.com/en/docs/hi3516a-1.html

海思arm板采集mac电脑的hdmi信号进行实时录屏直播：

编译：
cd Debug && make all


将编译出来的Hi3516a_hdmi_rtsp拷贝到海思板子上

将src/param.ini配置文件放在与Hi3516a_hdmi_rtsp可执行文件同一目录下。

拷贝程序依赖的库文件：


配置文件param.ini参数含义:

AEventCenterEnable=1  #是否开启aec功能，

appId=APPID-FREE

userId=   #第一次启动时请留空，否则可能会出现重复id



下载其它[客户端示例程序](https://docs.starrtc.com/en/download/)，进入互动直播，即可观看直播。

![arm_hdmi](https://raw.githubusercontent.com/starrtc/starrtc-android-demo/master/assets/arm_hdmi.jpg)

![arm_hdmi_screen](https://raw.githubusercontent.com/starrtc/starrtc-android-demo/master/assets/arm_hdmi_screen.jpg)

hdmi接摄像头：

![camera](https://raw.githubusercontent.com/starrtc/starrtc-android-demo/master/assets/camera.jpg)

Contact
=====
QQ ： 2162498688

邮箱：<a href="mailto:support@starRTC.com">support@starRTC.com</a>

手机: 186-1294-6552

微信：starRTC

QQ群：807242783