import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';

/*
static const uint16_t GATTS_CHAR_EQ_GAINS_VAL                       = 0xFF01;
static const uint16_t GATTS_CHAR_MIXER_INPUT_GAINS_VAL              = 0xFF02;
static const uint16_t GATTS_CHAR_MIXER_ENABLE_JACK_IN_VAL           = 0xFF03;
static const uint16_t GATTS_CHAR_MIXER_ENABLE_BLUETOOTH_A2DP_IN_VAL = 0xFF04;
static const uint16_t GATTS_CHAR_OUTPUT_GAIN_VAL                    = 0xFF05;
 */

class DeviceScreen extends StatefulWidget {
  DeviceScreen({Key? key, required this.device}) : super(key: key);

  final BluetoothDevice device;
  final Map<Guid, List<int>> readValues = {};
  List<double> gain = [0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0];
  List<double> mixerGains = [0.0, 0.0];

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
  //Lists services for BLE
  List<BluetoothService> _services = [];
  bool connected = true;
  bool enableBT = true;
  bool enableAUX = true;

  @override
  initState() {
    super.initState();
    _stateListener = widget.device.state.listen((event) {
      debugPrint('event :  $event');
      if (deviceState == event) {
        return;
      }
      setBleConnectionState(event);
    });
    enableBT = true;
    enableAUX = true;
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
        connected = false;
        stateText = 'Disconnected';
        connectButtonText = 'Connect';
        print('####2################## Disconnected ####################################');
        break;
      case BluetoothDeviceState.disconnecting:
        stateText = 'Disconnecting';
        break;
      case BluetoothDeviceState.connected:
        connected = true;
        stateText = 'Connected';
        connectButtonText = 'Disconnect';
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


  Future<bool> connect() async {
    // Future<bool>? returnValue;
    // setState(() {
    //   stateText = 'Connecting';
    //
    // });

    Future<bool>? returnValue;

    // try {
    //   await widget.device.connect(timeout: const Duration(seconds: 10));
    // } catch (e) {
    //   if (e != 'already_connected') {
    //     rethrow;
    //   }
    // } finally {
    //   _services = await widget.device.discoverServices();
    //   List<int> tempGain = await _services[2].characteristics[0].read();
    //
    //   //Converts casts values to int8
    //   for (int i = 0; i<10 ;i++) {
    //     int temp = tempGain[i].toSigned(8);
    //     widget.gain[i] = temp.toDouble();
    //   }
      //List <int> gainInts = widget.mixerGains.map((e) => e.toInt()).toList();

      /*print('Below is the value read from device');
      print(tempGain);
      //widget.gain = tempGain.map((e) => e.toDouble()).toList();
      print('Below is the value read from device in double');
      print(widget.gain);*/
    //}



/*    for (BluetoothService service in _services) {
      print('Service');
      print(service.uuid.toString());
      print('Char');
      for (BluetoothCharacteristic characteristic in service.characteristics) {
        print(characteristic.uuid.toString());
      }
    }*/

    setState(()  {
      stateText = 'Connecting';
      //_connectedDevice = widget.device;
      //_services[2].characteristics[2].write(statusAUX);
      //_services[2].characteristics[2].write(statusAUX);
    });

    await widget.device.connect(autoConnect: false).timeout(
        const Duration(milliseconds: 10000),
        onTimeout: () {
          returnValue = Future.value(false);
          debugPrint('timeout failed');
          setBleConnectionState(BluetoothDeviceState.disconnected);
        }).then((data) {
          if (returnValue == null) {
            debugPrint('connection successful');
            returnValue = Future.value(true);
          }
        });
    //Look for services and their charecteristics
    _services = await widget.device.discoverServices();

    //Read values of Equalizer and Mixer gains
    List<int> tempGain = await _services[2].characteristics[0].read();
    List<int> tempMixerGains = await _services[2].characteristics[1].read();

    //Converts casts values to int8
    for (int i = 0; i<10 ;i++) {
      int temp = tempGain[i].toSigned(8);
      widget.gain[i] = temp.toDouble();
      if (i < 2) {
        int temp1 = tempMixerGains[i].toSigned(8);
        widget.mixerGains[i] = temp1.toDouble();
      }
    }

    print('Below is the value read from device in double');
    print(widget.gain);

    setState(()  {
    });
    return returnValue ?? Future.value(false);
  }

  void disconnect() {
    try {
      setState(() {
        stateText = 'Disconnecting';
      });

      //Sends currect gain values from mixer and equalizer to device one last time
      List<int> gainEqualizerInts = widget.gain.map((e) => e.toInt()).toList();
      List<int> gainMixerInts = widget.gain.map((e) => e.toInt()).toList();
      _services[2].characteristics[0].write(gainEqualizerInts, withoutResponse: false);
      _services[2].characteristics[1].write(gainMixerInts, withoutResponse: false);
      widget.device.disconnect();
      print('###################### Disconnected ####################################');
    } catch (e) {
      print('Could not Disconnect');
    }
  }

  List<int> freq = [31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000];
  List<String> source = ['3.5mm Jack', 'Bluetooth'];
  List<int> statusBT = [1];
  List<int> statusAUX = [1];
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.device.name),
      ),
      body: SingleChildScrollView(
          scrollDirection: Axis.vertical,
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              const Divider(color: Color.fromRGBO(0, 0, 0, 0),height: 5.0),
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  const Spacer(),
                  Text(stateText),
                  Icon(connected ? Icons.bluetooth_connected : Icons.bluetooth_disabled),
                  const Spacer(),
                  const Spacer(),
                  const Spacer(),
                  OutlinedButton(
                      onPressed: () {
                        if (deviceState == BluetoothDeviceState.connected) {
                          disconnect();
                        } else if (deviceState == BluetoothDeviceState.disconnected) {
                          connect();
                        } else {}
                      },
                      child: Text(connectButtonText)
                  ),
                  const Spacer(),
                ],
              ),
              const Divider(color: Color.fromRGBO(0, 0, 0, 0),height: 5.0),
              const CustomDivider(text: 'Equalizer'),
              const Divider(color: Color.fromRGBO(0, 0, 0, 0),height: 25.0),
              SingleChildScrollView(
                  scrollDirection: Axis.horizontal,
                  child: Row(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: <Widget>[
                      //First
                      EqualizerSlider(index: 0, freq:freq, gain: widget.gain, service: _services, key: UniqueKey()),
                      EqualizerSlider(index: 1,freq:freq, gain: widget.gain, service: _services, key: UniqueKey()),
                      EqualizerSlider(index: 2,freq:freq, gain: widget.gain, service: _services, key: UniqueKey()),
                      EqualizerSlider(index: 3,freq:freq, gain: widget.gain, service: _services, key: UniqueKey()),
                      EqualizerSlider(index: 4,freq:freq, gain: widget.gain, service: _services, key: UniqueKey()),
                      EqualizerSlider(index: 5,freq:freq, gain: widget.gain, service: _services, key: UniqueKey()),
                      EqualizerSlider(index: 6,freq:freq, gain: widget.gain, service: _services, key: UniqueKey()),
                      EqualizerSlider(index: 7,freq:freq, gain: widget.gain, service: _services, key: UniqueKey()),
                      EqualizerSlider(index: 8,freq:freq, gain: widget.gain, service: _services, key: UniqueKey()),
                      EqualizerSlider(index: 9,freq:freq, gain: widget.gain, service: _services, key: UniqueKey()),
                    ],
                  )
              ),

              const Divider(color: Color.fromRGBO(0, 0, 0, 0),height: 25.0),
              const CustomDivider(text: 'Mixer'),
              const Divider(color: Color.fromRGBO(0, 0, 0, 0),height: 25.0),
              Row (
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  const Spacer(),
                  ElevatedButton(
                    style: ElevatedButton.styleFrom(
                      primary: enableAUX ? Colors.blue : Colors.transparent,
                      padding: const EdgeInsets.fromLTRB(5, 0, 5, 0)
                    ),
                    child: enableAUX ? const Text('Disable AUX Input') : const Text('Enable AUX Input'),
                    onPressed: () {
                      setState(() {
                        if (enableAUX) {
                          statusAUX[0] = 0;
                          _services[2].characteristics[2].write(statusAUX);
                        } else {
                          statusAUX[0] = 1;
                          _services[2].characteristics[2].write(statusAUX);
                        }
                        enableAUX = !enableAUX;
                      });
                    }
                  ),
                  const Spacer(),
                  ElevatedButton(
                      style: ElevatedButton.styleFrom(
                          primary: enableBT ? Colors.blue : Colors.transparent,
                          padding: const EdgeInsets.fromLTRB(5, 0, 5, 0)
                      ),
                      child: enableBT ? const Text('Disable Bluetooth Input') : const Text('Enable Bluetooth Input'),
                      onPressed: () {
                        setState(() {
                          if (enableBT) {
                            statusBT[0] = 0;
                            _services[2].characteristics[3].write(statusBT);
                          } else {
                            statusBT[0] = 1;
                            _services[2].characteristics[3].write(statusBT);
                          }
                          enableBT = !enableBT;
                        });
                      }
                  ),
                  const Spacer(),
                ],
              ),

              const Divider(color: Color.fromRGBO(0, 0, 0, 0),height: 25.0),
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  MixerSlider(index: 0, name: source, key: UniqueKey(),
                      mixerGains: widget.mixerGains, service: _services),
                  MixerSlider(index: 1, name: source, key: UniqueKey(),
                      mixerGains: widget.mixerGains, service: _services),
                ],
              ),

            ],
          )
      ),
    );
  }
}


class EqualizerSlider extends StatefulWidget {
  final int index;
  final List<int> freq;
  List<double> gain;
  List<BluetoothService> service;

  EqualizerSlider({Key? key, required this.index, required this.freq, required this.gain, required this.service}): super(key: key);

  /*
  //final String title;
  //const EqualizerSlider({Key? key, required this.title}) : super(key: key);
  //EqualizerSlider(this.freq, this.gain, this.title);
  */

  @override
  State<EqualizerSlider> createState() => _EqualizerSlider();
}


//https://stackoverflow.com/questions/58387596/i-want-to-ask-how-to-throw-the-value-of-the-slider-into-the-textfield-and-the-i
class _EqualizerSlider extends State<EqualizerSlider> {

  final myController = TextEditingController();

  List<int> gainInts = [];

  @override
  void initState() {
    super.initState();
    myController.addListener(_setSliderValue);
    //Gets value saved and loads it
    myController.text = widget.gain[widget.index].toInt().toString();
    _setSliderValue();
  }

  //Function called by listener when myController changes
  _setSliderValue () {
    setState(() {
      if (double.parse(myController.text).roundToDouble() < -20.0) {
        widget.gain[widget.index] = -20.0;
        myController.text = '-20';
      }
      else if (double.parse(myController.text).roundToDouble() > 10.0) {
        widget.gain[widget.index] = 10.0;
        myController.text = '10';
      }
      else {
        widget.gain[widget.index] = double.parse(myController.text).roundToDouble();
      }
      //Converts List double to List int
      gainInts = widget.gain.map((e) => e.toInt()).toList();

      //If services and gains are not empty
      if(gainInts.isNotEmpty && widget.service.isNotEmpty) {
        widget.service[2].characteristics[0].write(gainInts);
      }
      //print('Values: $gainInts');
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
                  myController.text = value.toInt().toString();
                });
              },
            ),
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


class MixerSlider extends StatefulWidget {
  final int index;
  final List<String> name;
  List<double> mixerGains;
  List<BluetoothService> service;

  MixerSlider({Key? key, required this.index, required this.name, required this.mixerGains, required this.service}): super(key: key);

  /*
  //final String title;
  //const MixerSlider({Key? key, required this.title}) : super(key: key);
  //MixerSlider(this.freq, this.gain, this.title);
  */

  @override
  State<MixerSlider> createState() => _MixerSlider();
}


//https://stackoverflow.com/questions/58387596/i-want-to-ask-how-to-throw-the-value-of-the-slider-into-the-textfield-and-the-i
class _MixerSlider extends State<MixerSlider> {

  final myController = TextEditingController();

  List<int> mixerGainsInt = [];

  @override
  void initState() {
    super.initState();
    myController.addListener(_setSliderValue);
    myController.text = widget.mixerGains[widget.index].toInt().toString();
    _setSliderValue();
  }

  //Function called by listener when myController changes
  _setSliderValue() {
    setState(() {
      //BluetoothCharacteristic characteristic = widget.service[0].characteristics[0];
      if (double.parse(myController.text).roundToDouble() < -20.0) {
        widget.mixerGains[widget.index] = -20.0;
        myController.text = '-20';
      }
      else if (double.parse(myController.text).roundToDouble() > 10.0) {
        widget.mixerGains[widget.index] = 10.0;
        myController.text = '10';
      }
      else {
        widget.mixerGains[widget.index] = double.parse(myController.text).roundToDouble();
      }

      //Converts List double to List int
      mixerGainsInt = widget.mixerGains.map((e) => e.toInt()).toList();

      //If services and gains are not empty
      if(mixerGainsInt.isNotEmpty && widget.service.isNotEmpty) {
        widget.service[2].characteristics[1].write(mixerGainsInt);
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisAlignment: MainAxisAlignment.center,
      crossAxisAlignment: CrossAxisAlignment.center,
      textBaseline: TextBaseline.alphabetic,
      children: <Widget>[
        //const Divider(color: null,height: 25.0),
        Text(
          widget.name[widget.index].toString(),
          style: const TextStyle(fontSize: 15.0, fontWeight: FontWeight.bold),
        ),
        //Widget for the slider
        Slider(
          value: widget.mixerGains[widget.index],
          max: 10,
          min: -20,

          divisions: 30,
          onChanged: (double value) {
            setState(() {
              widget.mixerGains[widget.index] = value;
              //Forces a call to _setSliderValue with change
              myController.text = value.toInt().toString();
            });
          },
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


class CustomDivider extends StatelessWidget {
  final String text;
  const CustomDivider({Key? key, required this.text}): super(key: key);
  
  @override
  Widget build(BuildContext context) {
  return Row(
    children: <Widget>[
      const Expanded(
        child: Divider()
      ),
      Text(text),
      const Expanded(
        child: Divider()
      ),
    ]
    );
  }
}