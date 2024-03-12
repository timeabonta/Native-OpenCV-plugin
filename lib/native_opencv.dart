import 'dart:async';
import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

// A C könyvtár betöltése
final DynamicLibrary nativeLib =
  Platform.isAndroid ? DynamicLibrary.open("libnative_opencv.so") : DynamicLibrary.process();

// C függvények aláírásai
typedef _c_version = Pointer<Utf8> Function();
typedef _c_processImage = Pointer<Utf8> Function(Pointer<Utf8> imagePath);

// Dart függvények aláírásai
typedef _dart_version = Pointer<Utf8> Function();
typedef _dart_processImage = Pointer<Utf8> Function(Pointer<Utf8> imagePath);

// Dart függvények létrehozása, amelyek meghívják a C függvényeket
final _version = nativeLib.lookupFunction<_c_version, _dart_version>('version');
final _processImage = nativeLib.lookupFunction<_c_processImage, _dart_processImage>('processImage');

class NativeOpencv {

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

  static Future<String> processImage(String imagePath) async {
    final imagePathPtr = imagePath.toNativeUtf8();
    final resultPtr = _processImage(imagePathPtr);
    final result = resultPtr.toDartString();
    calloc.free(imagePathPtr);
    return result;
  }

}

