import 'package:flutter/material.dart';

void main() {
  runApp(const MyApp());
}
class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        // This is the theme of your application.
        //
        // Try running your application with "flutter run". You'll see the
        // application has a blue toolbar. Then, without quitting the app, try
        // changing the primarySwatch below to Colors.green and then invoke
        // "hot reload" (press "r" in the console where you ran "flutter run",
        // or simply save your changes to "hot reload" in a Flutter IDE).
        // Notice that the counter didn't reset back to zero; the application
        // is not restarted.
        primarySwatch: Colors.blue,
      ),
      home: const MyHomePage(title: 'Equalizer'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key? key, required this.title}) : super(key: key);

  // This widget is the home page of your application. It is stateful, meaning
  // that it has a State object (defined below) that contains fields that affect
  // how it looks.

  // This class is the configuration for the state. It holds the values (in this
  // case the title) provided by the parent (in this case the App widget) and
  // used by the build method of the State. Fields in a Widget subclass are
  // always marked "final".

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();

}


class _MyHomePageState extends State<MyHomePage> {
  List<double> gain = [0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0];
  List<int> freq = [31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000];
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.title)
      ),
      body:  Row(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            //First
            CustomSlider(freq[0], gain[0]),/*
            CustomSlider(freq[1], gain[1]),
            CustomSlider(freq[2], gain[2]),
            CustomSlider(freq[3], gain[3]),
            CustomSlider(freq[4], gain[4]),
            CustomSlider(freq[5], gain[5]),
            CustomSlider(freq[6], gain[6]),
            CustomSlider(freq[7], gain[7]),
            CustomSlider(freq[8], gain[8]),
            CustomSlider(freq[9], gain[9]),*/
          ],
        )
      )
    ;
  }
}

class CustomSlider extends StatefulWidget {

  int freq;
  double gain;
  CustomSlider(this.freq, this.gain);

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

  @override
  void initState() {
    super.initState();
    myController.addListener(_setSliderValue);
  }

  //Function called by listener when myController changes
  _setSliderValue() {
    setState(() {
      if (double.parse(myController.text).roundToDouble() < -20.0) {
        widget.gain = -20.0;
        myController.text = '-20.0';
      }
      else if (double.parse(myController.text).roundToDouble() > 20.0) {
        widget.gain = 20.0;
        myController.text = '20.0';
      }
      else {
        widget.gain = double.parse(myController.text).roundToDouble();
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
        Text(
          widget.freq.toString(),
          style: const TextStyle(fontSize: 15.0, fontWeight: FontWeight.bold),
        ),
        const Text(
          'Hz',
          style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.bold),
        ),
        //Widget for the slider
        RotatedBox(
            quarterTurns: 3,
            child: Slider(
              value: widget.gain,
              max: 20,
              min: -20,
              divisions: 40,
              onChanged: (double value) {
                setState(() {
                  widget.gain = value;
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
                  hintText: ''
              ),
              style: const TextStyle(fontSize: 15.0,
              fontWeight: FontWeight.bold
              ),
              onSubmitted: (String value) {
                setState(() {
                  if (double.parse(value) < -20.0) {
                    widget.gain = -20.0;
                  }
                  else if (double.parse(value) > 20.0) {
                    widget.gain = 20.0;
                  }
                  else {
                    widget.gain = double.parse(value);
                  }
                });
              },
              )
        ),
        const Text(
          'dB',
          style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.bold),
        )
      ],
    );
  }
}
