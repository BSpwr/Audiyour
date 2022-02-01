
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