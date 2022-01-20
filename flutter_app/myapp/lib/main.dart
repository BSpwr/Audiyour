import 'package:flutter/material.dart';

void main() {
  runApp(const MyApp());
}
//Peer Help: Helped Enemias get acquainted with running the Flutter App on a virtual machine
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
      home: const MyHomePage(title: 'Flutter Demo Home Page'),
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
      body: RotatedBox(
        quarterTurns: 3,
        child: Row(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            //First
            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.center,
              textBaseline: TextBaseline.alphabetic,
              children: <Widget>[
                Text(
                  freq[0].toString(),
                  style: const TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                RotatedBox(
                    quarterTurns: 3,
                    child: Slider(
                      value: gain[0],
                      max: 20,
                      min: -20,
                      divisions: 40,
                      onChanged: (double value) {
                        setState(() {
                          gain[0] = value;
                        });
                      },
                    )
                ),


                Text(
                  gain[0].toString(),
                  style: const TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                Text("dB")
              ],
            ),

            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.center,
              textBaseline: TextBaseline.alphabetic,
              children: <Widget>[
                Text(
                  freq[1].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                RotatedBox(
                    quarterTurns: 3,
                    child: Slider(
                      value: gain[1],
                      max: 20,
                      min: -20,
                      divisions: 40,
                      onChanged: (double value) {
                        setState(() {
                          gain[1] = value;
                        });
                      },
                    )
                ),


                Text(
                  gain[1].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                Text("dB")
              ],
            ),

            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.center,
              textBaseline: TextBaseline.alphabetic,
              children: <Widget>[
                Text(
                  freq[2].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                RotatedBox(
                    quarterTurns: 3,
                    child: Slider(
                      value: gain[2],
                      max: 20,
                      min: -20,
                      divisions: 40,
                      onChanged: (double value) {
                        setState(() {
                          gain[2] = value;
                        });
                      },
                    )
                ),


                Text(
                  gain[2].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                Text("dB")
              ],
            ),

            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.center,
              textBaseline: TextBaseline.alphabetic,
              children: <Widget>[
                Text(
                  freq[3].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                RotatedBox(
                    quarterTurns: 3,
                    child: Slider(
                      value: gain[3],
                      max: 20,
                      min: -20,
                      divisions: 40,
                      onChanged: (double value) {
                        setState(() {
                          gain[3] = value;
                        });
                      },
                    )
                ),


                Text(
                  gain[3].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                Text("dB")
              ],
            ),

            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.center,
              textBaseline: TextBaseline.alphabetic,
              children: <Widget>[
                Text(
                  freq[4].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                RotatedBox(
                    quarterTurns: 3,
                    child: Slider(
                      value: gain[4],
                      max: 20,
                      min: -20,
                      divisions: 40,
                      onChanged: (double value) {
                        setState(() {
                          gain[4] = value;
                        });
                      },
                    )
                ),


                Text(
                  gain[4].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                Text("dB")
              ],
            ),

            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.center,
              textBaseline: TextBaseline.alphabetic,
              children: <Widget>[
                Text(
                  freq[5].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                RotatedBox(
                    quarterTurns: 3,
                    child: Slider(
                      value: gain[5],
                      max: 20,
                      min: -20,
                      divisions: 40,
                      onChanged: (double value) {
                        setState(() {
                          gain[5] = value;
                        });
                      },
                    )
                ),


                Text(
                  gain[5].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                Text("dB")
              ],
            ),

            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.center,
              textBaseline: TextBaseline.alphabetic,
              children: <Widget>[
                Text(
                  freq[6].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                RotatedBox(
                    quarterTurns: 3,
                    child: Slider(
                      value: gain[6],
                      max: 20,
                      min: -20,
                      divisions: 40,
                      onChanged: (double value) {
                        setState(() {
                          gain[6] = value;
                        });
                      },
                    )
                ),


                Text(
                  gain[6].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                Text("dB")
              ],
            ),


            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.center,
              textBaseline: TextBaseline.alphabetic,
              children: <Widget>[
                Text(
                  freq[7].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                RotatedBox(
                    quarterTurns: 3,
                    child: Slider(
                      value: gain[7],
                      max: 20,
                      min: -20,
                      divisions: 40,
                      onChanged: (double value) {
                        setState(() {
                          gain[7] = value;
                        });
                      },
                    )
                ),


                Text(
                  gain[7].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                Text("dB")
              ],
            ),

            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.center,
              textBaseline: TextBaseline.alphabetic,
              children: <Widget>[
                Text(
                  freq[8].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                RotatedBox(
                    quarterTurns: 3,
                    child: Slider(
                      value: gain[8],
                      max: 20,
                      min: -20,
                      divisions: 40,
                      onChanged: (double value) {
                        setState(() {
                          gain[8] = value;
                        });
                      },
                    )
                ),


                Text(
                  gain[8].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                Text("dB")
              ],
            ),

            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.center,
              textBaseline: TextBaseline.alphabetic,
              children: <Widget>[
                Text(
                  freq[9].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                RotatedBox(
                    quarterTurns: 3,
                    child: Slider(
                      value: gain[9],
                      max: 20,
                      min: -20,
                      divisions: 40,
                      onChanged: (double value) {
                        setState(() {
                          gain[9] = value;
                        });
                      },
                    )
                ),


                Text(
                  gain[9].toString(),
                  style: TextStyle(fontSize: 15.0, fontWeight: FontWeight.w900),
                ),
                Text("dB")
              ],
            )
          ],
        ),
      )
    );
  }
}
