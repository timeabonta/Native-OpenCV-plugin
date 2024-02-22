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

// Dart függvények aláírásai
typedef _dart_version = Pointer<Utf8> Function();

// Dart függvények létrehozása, amelyek meghívják a C függvényeket
final _version = nativeLib.lookupFunction<_c_version, _dart_version>('version');

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
}

