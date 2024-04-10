import 'dart:async';
import 'dart:ffi';
import 'dart:io';
import 'dart:typed_data';
import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

// A C könyvtár betöltése
final DynamicLibrary nativeLib =
  Platform.isAndroid ? DynamicLibrary.open("libnative_opencv.so") : DynamicLibrary.process();

// C függvények aláírásai
typedef _c_version = Pointer<Utf8> Function();
typedef _c_processImage = Pointer<Utf8> Function(Pointer<Utf8> imagePath);
typedef _c_detectAndFrameObject = Pointer<Utf8> Function(Pointer<Utf8> imagePath);
typedef _c_initDetector = Void Function(Int32 hue, Int32 hueTolerance);
typedef _c_destroyDetector = Void Function();
typedef _c_detect = Pointer<Float> Function(
    Int32 width, Int32 height, Int32 rotation, Pointer<Uint8> bytes, Bool isYUV, Pointer<Int32> outCount);

// Dart függvények aláírásai
typedef _dart_version = Pointer<Utf8> Function();
typedef _dart_processImage = Pointer<Utf8> Function(Pointer<Utf8> imagePath);
typedef _dart_detectAndFrameObject = Pointer<Utf8> Function(Pointer<Utf8> imagePath);
typedef _dart_initDetector = void Function(int hue, int hueTolerance);
typedef _dart_destroyDetector = void Function();
typedef _dart_detect = Pointer<Float> Function(
    int width, int height, int rotation, Pointer<Uint8> bytes, bool isYUV, Pointer<Int32> outCount);

// Dart függvények létrehozása, amelyek meghívják a C függvényeket
final _version = nativeLib.lookupFunction<_c_version, _dart_version>('version');
final _processImage = nativeLib.lookupFunction<_c_processImage, _dart_processImage>('processImage');
final _detectAndFrameObject = nativeLib.lookupFunction<_c_detectAndFrameObject, _dart_detectAndFrameObject>('detectAndFrameObjects');
final _initDetector = nativeLib.lookupFunction<_c_initDetector, _dart_initDetector>('initDetector');
final _destroyDetector = nativeLib.lookupFunction<_c_destroyDetector, _dart_destroyDetector>('destroyDetector');
final _detect = nativeLib.lookupFunction<_c_detect, _dart_detect>('detect');

class DetectedObject {
  File imageFile;
  int dominantHue;

  DetectedObject(this.imageFile, this.dominantHue);
}

class NativeOpencv {
  Pointer<Uint8>? _imageBuffer;

  static final DynamicLibrary nativeLib = Platform.isAndroid
      ? DynamicLibrary.open("libnative_opencv.so")
      : DynamicLibrary.process();

  static final _dart_detectAndFrameObject detectAndFrameObject = nativeLib
      .lookupFunction<_c_detectAndFrameObject, _dart_detectAndFrameObject>('detectAndFrameObject');


  static const MethodChannel _channel = MethodChannel('native_opencv');
  static Future<String?> get platformVersion async {
    final String? version =
    await _channel.invokeMethod('getPlatformVersion'); // A platform verzió lekérdezése
    return version;
  }

  // OpenCV verziójának lekérdezése
  String cvVersion() {
    return _version().toDartString();
  }

  static Future<List<DetectedObject>?> detectAndFrameObjects(String imagePath) async {
    final imagePathPtr = imagePath.toNativeUtf8();
    final resultPtr = _detectAndFrameObject(imagePathPtr);
    calloc.free(imagePathPtr);
    final result = resultPtr.toDartString();
    if (result.startsWith('Failed')) {
      print(result);
      return null;
    }
    final entries = result.split(';');
    List<DetectedObject> objects = [];
    for (String entry in entries) {
      if (entry.isNotEmpty) {
        final parts = entry.split(',');
        if (parts.length == 2) {
          objects.add(DetectedObject(File(parts[0]), int.parse(parts[1])));
        }
      }
    }
    return objects;
  }

  void initDetector(int hue, int hueTolerance) {
    _initDetector(hue, hueTolerance);
  }

  void destroy() {
    _destroyDetector();
    if (_imageBuffer != null) {
      malloc.free(_imageBuffer!);
      _imageBuffer = null;
    }
  }

  Float32List detect(int width, int height, int rotation, Uint8List yBuffer, Uint8List? uBuffer, Uint8List? vBuffer) {
    var ySize = yBuffer.lengthInBytes;
    var uSize = uBuffer?.lengthInBytes ?? 0;
    var vSize = vBuffer?.lengthInBytes ?? 0;
    var totalSize = ySize + uSize + vSize;

    _imageBuffer ??= malloc.allocate<Uint8>(totalSize);

    Uint8List _bytes = _imageBuffer!.asTypedList(totalSize);
    _bytes.setAll(0, yBuffer);

    if (Platform.isAndroid) {
      _bytes.setAll(ySize, vBuffer!);
      _bytes.setAll(ySize + vSize, uBuffer!);
    }

    Pointer<Int32> outCount = malloc.allocate<Int32>(1);
    var res = _detect(width, height, rotation, _imageBuffer!, Platform.isAndroid ? true : false, outCount);
    final count = outCount.value;

    malloc.free(outCount);
    return res.asTypedList(count);
  }
}
