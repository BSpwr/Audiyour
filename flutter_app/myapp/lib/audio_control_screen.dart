import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';

class DeviceScreen extends StatefulWidget {
  DeviceScreen({Key? key, required this.device}) : super(key: key);

  final BluetoothDevice device;
  final Map<Guid, List<int>> readValues = {};
  List<double> gain = [0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0];



  @override
  _DeviceScreenState createState() => _DeviceScreenState();
}

class _DeviceScreenState extends State<DeviceScreen> {
  // Flutter Blue instance
  FlutterBlue flutterBlue = FlutterBlue.instance;

  String stateText = 'Connecting';
  String connectButtonText = 'Disconnect';

  //Defaults Bluetooth state is disconnected
  BluetoothDeviceState deviceState = BluetoothDeviceState.disconnected;

  //
  StreamSubscription<BluetoothDeviceState>? _stateListener;
  final _writeController = TextEditingController();
  //Lists services for BLE
  List<BluetoothService> _services = [];
  BluetoothDevice? _connectedDevice;
  bool connected = true;

  @override
  initState() {
    super.initState();
    // _stateListener = widget.device.state.listen((event) {
    //   debugPrint('event :  $event');
    //   if (deviceState == event) {
    //     return;
    //   }
    //   setBleConnectionState(event);
    // });
    connect();

  }

  @override
  void dispose() {
    _stateListener?.cancel();
    disconnect();
    super.dispose();
  }

  @override
  void setState(VoidCallback fn) {
    if (mounted) {
      super.setState(fn);
    }
  }

  setBleConnectionState(BluetoothDeviceState event) {
    switch (event) {
      case BluetoothDeviceState.disconnected:
        stateText = 'Disconnected';
        // 버튼 상태 변경
        connectButtonText = 'Connect';
        connected = false;
        break;
      case BluetoothDeviceState.disconnecting:
        stateText = 'Disconnecting';
        break;
      case BluetoothDeviceState.connected:
        stateText = 'Connected';
        connectButtonText = 'Disconnect';
        connected = true;
        break;
      case BluetoothDeviceState.connecting:
        stateText = 'Connecting';
        break;
    }
    deviceState = event;
    setState(() {});
  }

  iconPress (){
    if (deviceState == BluetoothDeviceState.connected) {
      disconnect();
    } else if (deviceState == BluetoothDeviceState.disconnected) {
      connect();
    } else {}
  }


  //Future<bool>
  connect() async {
    // Future<bool>? returnValue;
    // setState(() {
    //   stateText = 'Connecting';
    //
    // });

    try {
      await widget.device.connect(timeout: const Duration(seconds: 10));
    } catch (e) {
      if (e != 'already_connected') {
        throw e;
      }
    } finally {
      _services = await widget.device.discoverServices();
    }

    setState(() {
      _connectedDevice = widget.device;
    });

    for (BluetoothService service in _services) {
      print('Service');
      print(service.uuid.toString());
      print('Char');
      for (BluetoothCharacteristic characteristic in service.characteristics) {
        print(characteristic.uuid.toString());
      }
    }

    // await widget.device.connect(autoConnect: false).timeout(
    //     Duration(milliseconds: 10000),
    //     onTimeout: () {
    //       returnValue = Future.value(false);
    //       debugPrint('timeout failed');
    //       setBleConnectionState(BluetoothDeviceState.disconnected);
    //     }).then((data) {
    //       if (returnValue == null) {
    //         debugPrint('connection successful');
    //         returnValue = Future.value(true);
    //       }
    //     });
    // _services = await widget.device.discoverServices();

    //return returnValue ?? Future.value(false);
  }

  void disconnect() {
    try {
      setState(() {
        stateText = 'Disconnecting';
      });
      widget.device.disconnect();
    } catch (e) {}
  }

  //widget.gain = [0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0];
  List<int> freq = [31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      floatingActionButton: FloatingActionButton(
        onPressed: iconPress,
        //If scanning a stop is shown, else a search icon
        child: Icon(connected ? Icons.bluetooth_connected : Icons.bluetooth_disabled),
      ),
      appBar: AppBar(
        title: Text(widget.device.name),
      ),
      body: Center(
            child: SingleChildScrollView(
                scrollDirection: Axis.vertical,
                child: SingleChildScrollView(
                    scrollDirection: Axis.horizontal,
                    child: Row(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: <Widget>[
                        //First
                        CustomSlider(index: 0, freq:freq, gain: widget.gain, service: _services, key: UniqueKey()),
                        CustomSlider(index: 1,freq:freq, gain: widget.gain, service: _services),
                        CustomSlider(index: 2,freq:freq, gain: widget.gain, service: _services),
                        CustomSlider(index: 3,freq:freq, gain: widget.gain, service: _services),
                        CustomSlider(index: 4,freq:freq, gain: widget.gain, service: _services),
                        CustomSlider(index: 5,freq:freq, gain: widget.gain, service: _services),
                        CustomSlider(index: 6,freq:freq, gain: widget.gain, service: _services),
                        CustomSlider(index: 7,freq:freq, gain: widget.gain, service: _services),
                        CustomSlider(index: 8,freq:freq, gain: widget.gain, service: _services),
                        CustomSlider(index: 9,freq:freq, gain: widget.gain, service: _services),
                      ],
                    )
                ),
            ),
      ),
    );
  }
}


class CustomSlider extends StatefulWidget {
  final int index;
  final List<int> freq;
  List<double> gain;
  List<BluetoothService> service;

  CustomSlider({Key? key, required this.index, required this.freq, required this.gain, required this.service}): super(key: key);

  /*
  //final String title;
  //const CustomSlider({Key? key, required this.title}) : super(key: key);
  //CustomSlider(this.freq, this.gain, this.title);
  */

  @override
  State<CustomSlider> createState() => _CustomSlider();
}


//https://stackoverflow.com/questions/58387596/i-want-to-ask-how-to-throw-the-value-of-the-slider-into-the-textfield-and-the-i
class _CustomSlider extends State<CustomSlider> {

  final myController = TextEditingController();

  List<int> gainInts = [];

  @override
  void initState() {
    super.initState();
    myController.addListener(_setSliderValue);
  }

  //Function called by listener when myController changes
  _setSliderValue() {
    setState(() {
      //BluetoothCharacteristic characteristic = widget.service[0].characteristics[0];
      if (double.parse(myController.text).roundToDouble() < -20.0) {
        widget.gain[widget.index] = -20.0;
        myController.text = '-20.0';
      }
      else if (double.parse(myController.text).roundToDouble() > 10.0) {
        widget.gain[widget.index] = 10.0;
        myController.text = '10.0';
      }
      else {
        widget.gain[widget.index] = double.parse(myController.text).roundToDouble();
      }

      gainInts = widget.gain.map((e) => e.toInt()).toList();
      print(gainInts);
      print('data');
      print(widget.service[2].uuid.toString());
      print(widget.service[2].characteristics[0].uuid.toString());
      widget.service[2].characteristics[0].write(gainInts);

      //print(widget.service[0].characteristics[0].uuid.hashCode);
      //characteristic.write(gainInts);
    });
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisAlignment: MainAxisAlignment.center,
      crossAxisAlignment: CrossAxisAlignment.center,
      textBaseline: TextBaseline.alphabetic,
      children: <Widget>[
        Text(
          widget.freq[widget.index].toString(),
          style: const TextStyle(fontSize: 15.0, fontWeight: FontWeight.bold),
        ),
        const Text('Hz', style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.bold),
        ),

        //Widget for the slider
        RotatedBox(
            quarterTurns: 3,
            child: Slider(
              value: widget.gain[widget.index],
              max: 10,
              min: -20,
              divisions: 30,
              onChanged: (double value) {
                setState(() {
                  widget.gain[widget.index] = value;
                  //Forces a call to _setSliderValue with change
                  myController.text = value.toString();
                });
              },
            )
        ),

        //Widget for the Text Box
        SizedBox(
            width: 40.0,
            height: 20,
            child: TextField(
              textAlignVertical: TextAlignVertical.center,
              textAlign: TextAlign.center,
              controller: myController,
              decoration: const InputDecoration.collapsed (border: OutlineInputBorder(),
                  hintText: '0.0'
              ),
              style: const TextStyle(fontSize: 15.0,
                fontWeight: FontWeight.bold
              ),
              )
        ),
        const Text('dB', style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.bold),
        )
      ],
    );
  }
}
