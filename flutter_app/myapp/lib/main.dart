
import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';

import 'audio_control_screen.dart';

void main() {
  runApp(const MyApp());
}
class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: const MyHomePage(title: 'Audiyour (BLE Connection)'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key? key, required this.title}) : super(key: key);
  final String title;

  @override
  _MyHomePageState createState() => _MyHomePageState();
}
//https://github.com/dong-higenis/flutter_ble_scan_example/tree/main/example_3/lib
class _MyHomePageState extends State<MyHomePage> {

  //Creating instance
  FlutterBlue flutterBlue = FlutterBlue.instance;
  //List that holds BLE options
  List<ScanResult> scanResultList = [];
  //If scanning
  bool _isScanning = false;

  @override
  void initState() {
    super.initState();
    initBle();
  }

  void initBle() {
    //If BLE is scanning true is saved
    flutterBlue.isScanning.listen((isScanning) {
      _isScanning = isScanning;
      setState(() {});
    });
  }

  scan() async {
    if (!_isScanning) {
      //Clears list of BLE connections on screen
      scanResultList.clear();
      //Scans asynchronously for 4 seconds
      flutterBlue.startScan(timeout: const Duration(seconds: 4));
      //Lists all devices that are found
      flutterBlue.scanResults.listen((results) {
        scanResultList = results;
        setState(() {});
      });
    } else {
      flutterBlue.stopScan();
    }
  }

  //Returns text widget of signal strength
  Widget deviceSignal(ScanResult res) {
    return Text(res.rssi.toString());
  }

  //Returns text widget of MAC address
  Widget deviceMacAddress(ScanResult res) {
    return Text(res.device.id.id);
  }

  //Returns text widget of device name
  Widget deviceName(ScanResult res) {
    if (res.device.name.isNotEmpty) {
      return Text(res.device.name);
    } else if (res.advertisementData.localName.isNotEmpty) {
      return Text(res.advertisementData.localName);
    } else {
      return const Text('N/A');
    }
  }

  Widget leading(ScanResult res) {
    return const CircleAvatar(
      child: Icon(
        Icons.bluetooth,
        color: Colors.white,
      ),
      backgroundColor: Colors.cyan,
    );
  }

  void onTap(ScanResult r) {
    //Moves screen to equalizer window
    Navigator.push(context,
      MaterialPageRoute(builder: (context) => DeviceScreen(device: r.device)),
    );
  }

  //Lists BLE connections with Name, MAC address, and signals
  Widget listItem(ScanResult r) {
    return ListTile(
      onTap: () => onTap(r),
      leading: leading(r),
      title: deviceName(r),
      subtitle: deviceMacAddress(r),
      trailing: deviceSignal(r),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.title),
      ),
      body: Center(
        //Creates a list for each entry
        child: ListView.separated(
          itemCount: scanResultList.length,
          itemBuilder: (context, index) {
            return listItem(scanResultList[index]);
          },
          separatorBuilder: (context, index) {
            return const Divider();
          },
        ),
      ),
      //Button on the bottom right
      floatingActionButton: FloatingActionButton(
        onPressed: scan,
        //If scanning a stop is shown, else a search icon
        child: Icon(_isScanning ? Icons.stop : Icons.search),
      ),
    );
  }
}

// class _MyHomePageState extends State<MyHomePage> {
//   List<double> gain = [0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0];
//   List<int> freq = [31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000];
//   @override
//   Widget build(BuildContext context) {
//     return Scaffold(
//       appBar: AppBar(
//         title: Text(widget.title)
//       ),
//       body:  Center(
//         child: SingleChildScrollView(
//           scrollDirection: Axis.vertical,
//           child: SingleChildScrollView(
//               scrollDirection: Axis.horizontal,
//               child: Row(
//                 mainAxisAlignment: MainAxisAlignment.center,
//                 children: <Widget>[
//                   //First
//                   CustomSlider(freq:freq[0], gain: gain[0]),
//                   CustomSlider(freq:freq[1], gain: gain[1]),
//                   CustomSlider(freq:freq[2], gain:gain[2]),
//                   CustomSlider(freq:freq[3], gain:gain[3]),
//                   CustomSlider(freq:freq[4], gain:gain[4]),
//                   CustomSlider(freq:freq[5], gain:gain[5]),
//                   CustomSlider(freq:freq[6], gain:gain[6]),
//                   CustomSlider(freq:freq[7], gain:gain[7]),
//                   CustomSlider(freq:freq[8], gain:gain[8]),
//                   CustomSlider(freq:freq[9], gain:gain[9]),
//                 ],
//               )
//           ),
//         ),
//       )
//       );
//   }
// }
//
// class CustomSlider extends StatefulWidget {
//
//   final int freq;
//   double gain;
//
//   CustomSlider({Key? key, required this.freq, required this.gain}): super(key: key);
//
//   /*
//   //final String title;
//   //const CustomSlider({Key? key, required this.title}) : super(key: key);
//   //CustomSlider(this.freq, this.gain, this.title);
//   */
//
//   @override
//   State<CustomSlider> createState() => _CustomSlider();
// }
//
//
// //https://stackoverflow.com/questions/58387596/i-want-to-ask-how-to-throw-the-value-of-the-slider-into-the-textfield-and-the-i
// class _CustomSlider extends State<CustomSlider> {
//
//   final myController = TextEditingController();
//
//   @override
//   void initState() {
//     super.initState();
//     myController.addListener(_setSliderValue);
//   }
//
//   //Function called by listener when myController changes
//   _setSliderValue() {
//     setState(() {
//       if (double.parse(myController.text).roundToDouble() < -20.0) {
//         widget.gain = -20.0;
//         myController.text = '-20.0';
//       }
//       else if (double.parse(myController.text).roundToDouble() > 20.0) {
//         widget.gain = 20.0;
//         myController.text = '20.0';
//       }
//       else {
//         widget.gain = double.parse(myController.text).roundToDouble();
//       }
//     });
//   }
//
//   @override
//   Widget build(BuildContext context) {
//     return Column(
//       mainAxisAlignment: MainAxisAlignment.center,
//       crossAxisAlignment: CrossAxisAlignment.center,
//       textBaseline: TextBaseline.alphabetic,
//       children: <Widget>[
//         Text(
//           widget.freq.toString(),
//           style: const TextStyle(fontSize: 15.0, fontWeight: FontWeight.bold),
//         ),
//         const Text('Hz', style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.bold),
//         ),
//
//         //Widget for the slider
//         RotatedBox(
//             quarterTurns: 3,
//             child: Slider(
//               value: widget.gain,
//               max: 20,
//               min: -20,
//               divisions: 40,
//               onChanged: (double value) {
//                 setState(() {
//                   widget.gain = value;
//                   //Forces a call to _setSliderValue with change
//                   myController.text = value.toString();
//                 });
//               },
//             )
//         ),
//
//         //Widget for the Text Box
//         SizedBox(
//             width: 40.0,
//             height: 20,
//             child: TextField(
//               textAlignVertical: TextAlignVertical.center,
//               textAlign: TextAlign.center,
//               controller: myController,
//               decoration: const InputDecoration.collapsed (border: OutlineInputBorder(),
//                   hintText: '0.0'
//               ),
//               style: const TextStyle(fontSize: 15.0,
//                 fontWeight: FontWeight.bold
//               ),
//               )
//         ),
//         const Text('dB', style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.bold),
//         )
//       ],
//     );
//   }
// }
