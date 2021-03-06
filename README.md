# Capturer
`Capturer`是使用`Qt`开发的一款**截图**、**录屏**和**录制GIF**软件，支持`Windows`和`Linux`系统。
> `录屏`和`录制GIF`依赖于`FFmpeg`

## Download
https://github.com/ffiirree/Capturer/releases

## 默认快捷键
### 截图
 - `F1` : 截图
> 在截图时，使用`Ctrl + C`可以取色，保存在粘贴板中

### 贴图
 - `F3` : 将粘贴板中的内容作为图片贴出(文本内容也会渲染为图片)，如果粘贴板中的路径(路径为文本)为图片，则会贴出该图片
 - 鼠标滚轮 : 缩放贴图
 - `Ctrl` + 鼠标滚轮 ：更改贴图透明度
 - 鼠标左键双击 : 缩略图模式，贴图显示中心区域125x125的内容
 - 拖拽 : 拖拽图片到贴图上，则打开并显示拖拽图片
 - `ESC` : 关闭贴图窗

### 录屏
 - `Ctrl + Alt + V`第一次，开始选择区域，**回车**后开始录屏
 - `Ctrl + Alt + V`第二次，结束，视频保存在操作系统默认的`视频`文件夹

### 录制GIF
 - `Ctrl + Alt + G`第一次，开始选择区域，**回车**后开始录屏
 - `Ctrl + Alt + G`第一次，结束，GIF保存在操作系统默认的`图片`文件夹

### 选择框通用快捷键
 - `W / A / S / D`              : (逐像素)移动选中区域
 - `Ctrl + UP/DOWN/LEFT/RIGHT`  : (逐像素)扩大边界
 - `Shift + UP/DOWN/LEFT/RIGHT` : (逐像素)缩小边界
 - `Ctrl + A`                   : 全屏
 - `ESC`                        : 退出

## 源码使用
### 源代码下载
```
git clone https://github.com/ffiirree/Capturer.git
git submodule init
git submodule update
```
### 安装FFmpeg
#### Windows
从[官网](https://ffmpeg.zeranoe.com/builds/)下载编译好的二进制文件。

#### Ubuntu
```
sudo apt install ffmpeg
```

## 效果
![image](/capturer.png)
