# hiraishi2

個人的に制作しているレンダラーです．

様々なサイトや論文，書籍を参考にして制作しています．

hiraishiという名前は地元の地名からなんとなく取ったものです．

## Gallery

![img0](https://user-images.githubusercontent.com/27540771/94753496-60117a00-03c9-11eb-8749-fa4dc2506d5a.png)
![img1](https://user-images.githubusercontent.com/27540771/94753501-630c6a80-03c9-11eb-931d-c799b8d4bb8a.png)
![img2](https://user-images.githubusercontent.com/27540771/94753512-67d11e80-03c9-11eb-9314-7e0e10140664.png)

Model:Cornell Box downloaded from Morgan McGuire's [Computer Graphics Archive](https://casual-effects.com/data)

Model:Armadillo downloaded from [The Stanford 3D Scanning Repository](http://graphics.stanford.edu/data/3Dscanrep/)

## 機能
以下には簡易的だったり不完全なものを含みます
* レンダリング手法
    * Path Tracing
    * Next Event Estimation
    * Volume Rendering (Homogeneous media, single scattering)
    * Spectral Rendering
    * OpenGLによる簡易プレビュー
* マテリアル
    * Diffuse (Cosine-weighted)
    * Specular Microfacet BRDF [Walter, 2007]
    * Fresnel Reflection (Schlick's approximation)
    * Refract (Snell's law)
* アクセラレーション構造
    * Kd-Tree
* レイと三角形の交差判定
    * Möller–Trumbore intersection algorithm [Möller and Trumbore, 1997]
* 並列化
    * OpenMPでのレンダリング部分の並列化
* ポストプロセス
    * Averaging Filter
    * Gaussian Filter
    * Median Filter
    * Bilateral Filter
    * Joint Bilateral Filter (with Normal, Depth, Visibility and Albedo)
* 入出力
    * .obj .mtl の読み込み
    * .ppm での画像書き出し

## 開発環境
* C++
* Visual Studio Community 2019 16.7.5
* Windows 10 SDK 10.0.18362.0

## 注意
モデルなどはこのリポジトリに含まれません

## 参考文献
##### 論文
* [Kajiya1986] "The Rendering Equation"
* [Veach1997] "Robust Monte Carlo Methods for Light Transport Simulation"
* [Möller-Trumbore1997] "Fast, minimum storage ray-triangle intersection"
* [Walter2007] "Microfacet Models for Refraction through Rough Surfaces"
* [Fong2017] "Production Volume Rendering"

##### 書籍
M, Pharr et al., "Physically Based Rendering: From Theory to Implementation"
