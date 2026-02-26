<div align="center">

# MY-VIDEO-PLAYER

FFmpeg+CPP

</div>

<p align="center">
<img alt="GitHub License" src="https://img.shields.io/github/license/Bannirui/my-video-player">
<img alt="GitHub repo size" src="https://img.shields.io/github/repo-size/Bannirui/my-video-player">
<img alt="GitHub commit activity (branch)" src="https://img.shields.io/github/commit-activity/w/Bannirui/my-video-player">
<img alt="GitHub last commit (branch)" src="https://img.shields.io/github/last-commit/Bannirui/my-video-player">
</p>

<p align="center">
<a href="TODO.md">Todo</a>
</p>

## 1 Requirement

- Install FFmpeg

  ```sh
  sudo apt install ffmpeg libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libswresample-dev
  ```

- Install Qt
  
  ```sh
  sudo apt install qtmultimedia5-dev
  ```
  
```shell
ffmpeg -i Python.mp4 -f s16le Python.pcm
ffmpeg -i Python.mp4 -t 10 -s 240x128 -pix_fmt yuv420p Python.yuv
```
