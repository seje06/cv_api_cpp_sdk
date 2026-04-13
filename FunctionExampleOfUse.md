## 지원하는 이미지 처리 함수

아래 함수들은 `ProcessImageRequest`의 `operations`에 추가해서 사용하는 방식임.

기본 사용 형태는 아래와 같음.

```cpp
image_api::ProcessImageRequest request;
request.imageUrl = "https://example.com/test.jpg";
request.outputFormat = "png";

request.operations.push_back(...);
```

---

### `MakeGrayscale()`

컬러 이미지를 흑백 이미지로 변환하기 위한 함수.

#### 사용 예시

```cpp
request.operations.push_back(image_api::MakeGrayscale());
```

---

### `MakeInvert()`

이미지 색상을 반전하기 위한 함수.

#### 사용 예시

```cpp
request.operations.push_back(image_api::MakeInvert());
```

---

### `MakeBlur(int ksize, double sigma)`

가우시안 블러 적용용 함수.

#### 매개변수

- `ksize`
  - 블러 커널 크기
- `sigma`
  - 블러 강도에 영향을 주는 표준편차 값

#### 사용 예시

```cpp
request.operations.push_back(image_api::MakeBlur(9, 2.0));
```

---

### `MakeMedianBlur(int ksize)`

중간값 블러 적용용 함수.  
salt-and-pepper noise 제거 등에 사용 가능.

#### 매개변수

- `ksize`
  - 커널 크기

#### 사용 예시

```cpp
request.operations.push_back(image_api::MakeMedianBlur(5));
```

---

### `MakeBilateralFilter(int d, double sigmaColor, double sigmaSpace)`

엣지는 비교적 유지하면서 노이즈를 줄이기 위한 양방향 필터 함수.

#### 매개변수

- `d`
  - 필터링에 사용할 주변 픽셀 크기
- `sigmaColor`
  - 색상 공간 sigma
- `sigmaSpace`
  - 좌표 공간 sigma

#### 사용 예시

```cpp
request.operations.push_back(image_api::MakeBilateralFilter(9, 75.0, 75.0));
```

---

### `MakeSharpen()`

샤프닝 필터 적용용 함수.

#### 사용 예시

```cpp
request.operations.push_back(image_api::MakeSharpen());
```

---

### `MakeCannyEdge(double threshold1, double threshold2)`

캐니 엣지 검출용 함수.

#### 매개변수

- `threshold1`
  - 첫 번째 임계값
- `threshold2`
  - 두 번째 임계값

#### 사용 예시

```cpp
request.operations.push_back(image_api::MakeCannyEdge(80.0, 160.0));
```

---

### `MakeThreshold(double thresh, double maxValue, int thresholdType = 0)`

이진화 처리용 함수.

#### 매개변수

- `thresh`
  - 기준 임계값
- `maxValue`
  - threshold를 넘는 픽셀에 적용할 최대값
- `thresholdType`
  - threshold 타입 정수값

#### `thresholdType` 예시

- `0` : THRESH_BINARY
- `1` : THRESH_BINARY_INV
- `2` : THRESH_TRUNC
- `3` : THRESH_TOZERO
- `4` : THRESH_TOZERO_INV

#### 사용 예시

```cpp
request.operations.push_back(image_api::MakeThreshold(127.0, 255.0));
```

```cpp
request.operations.push_back(image_api::MakeThreshold(127.0, 255.0, 1));
```

---

### `MakeResize(int width, int height, int interpolation = 1)`

이미지 크기 변경용 함수.

#### 매개변수

- `width`
  - 목표 너비
- `height`
  - 목표 높이
- `interpolation`
  - 보간법

#### `interpolation` 예시

- `0` : INTER_NEAREST
- `1` : INTER_LINEAR
- `2` : INTER_CUBIC
- `3` : INTER_AREA

#### 사용 예시

```cpp
request.operations.push_back(image_api::MakeResize(512, 512));
```

```cpp
request.operations.push_back(image_api::MakeResize(512, 512, 2));
```

---

### `MakeCrop(int x, int y, int width, int height)`

이미지 일부 영역 자르기용 함수.

#### 매개변수

- `x`
  - 시작 x 좌표
- `y`
  - 시작 y 좌표
- `width`
  - 자를 영역 너비
- `height`
  - 자를 영역 높이

#### 사용 예시

```cpp
request.operations.push_back(image_api::MakeCrop(100, 50, 300, 200));
```

---

### `MakeFlip(int mode)`

이미지 반전용 함수.

#### 매개변수

- `mode`
  - `1` : 좌우 반전
  - `0` : 상하 반전
  - `-1` : 좌우 + 상하 모두 반전

#### 사용 예시

```cpp
request.operations.push_back(image_api::MakeFlip(1));
```

```cpp
request.operations.push_back(image_api::MakeFlip(0));
```

```cpp
request.operations.push_back(image_api::MakeFlip(-1));
```

---

### `MakeRotate(int angle)`

이미지 회전용 함수.

#### 매개변수

- `angle`
  - `90`
  - `180`
  - `270`

#### 사용 예시

```cpp
request.operations.push_back(image_api::MakeRotate(90));
```

```cpp
request.operations.push_back(image_api::MakeRotate(180));
```

---

### `MakeBrightnessContrast(double alpha, double beta)`

밝기 / 대비 조정용 함수.

#### 매개변수

- `alpha`
  - 대비 계수
- `beta`
  - 밝기 보정값

#### 사용 예시

```cpp
request.operations.push_back(image_api::MakeBrightnessContrast(1.2, 20.0));
```

```cpp
request.operations.push_back(image_api::MakeBrightnessContrast(0.8, -10.0));
```

---

### `MakeErode(int ksize = 3, int iterations = 1)`

침식 연산용 함수.

#### 매개변수

- `ksize`
  - 커널 크기
- `iterations`
  - 반복 횟수

#### 사용 예시

```cpp
request.operations.push_back(image_api::MakeErode());
```

```cpp
request.operations.push_back(image_api::MakeErode(5, 2));
```

---

### `MakeDilate(int ksize = 3, int iterations = 1)`

팽창 연산용 함수.

#### 매개변수

- `ksize`
  - 커널 크기
- `iterations`
  - 반복 횟수

#### 사용 예시

```cpp
request.operations.push_back(image_api::MakeDilate());
```

```cpp
request.operations.push_back(image_api::MakeDilate(5, 2));
```

---

## 전체 사용 예시

아래는 여러 연산을 순서대로 적용하는 예시.

```cpp
image_api::ProcessImageRequest request;
request.imageUrl = "https://example.com/test.jpg";
request.outputFormat = "png";

request.operations.push_back(image_api::MakeGrayscale());
request.operations.push_back(image_api::MakeBlur(9, 2.0));
request.operations.push_back(image_api::MakeThreshold(127.0, 255.0));
request.operations.push_back(image_api::MakeResize(512, 512));
```

아래는 기하 변환과 후처리를 같이 사용하는 예시.

```cpp
image_api::ProcessImageRequest request;
request.imageUrl = "https://example.com/test.jpg";
request.outputFormat = "jpg";

request.operations.push_back(image_api::MakeCrop(100, 50, 300, 200));
request.operations.push_back(image_api::MakeFlip(1));
request.operations.push_back(image_api::MakeRotate(90));
request.operations.push_back(image_api::MakeBrightnessContrast(1.1, 15.0));
request.operations.push_back(image_api::MakeSharpen());
```
