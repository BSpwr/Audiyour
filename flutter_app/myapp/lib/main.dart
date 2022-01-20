import 'package:flutter/material.dart';

void main() => runApp(MaterialApp(
  home: Home(),
));


class Home extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('my first app'),
        centerTitle: true,
      ),
      body: Center(
        child: IconButton(
          onPressed: () {
            print('your clicked me');
          },
          icon: Icon(Icons.alternate_email),
          color: Colors.amber
        )
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: () {},
        child: Text('click'),
      ),
    )
    ;
  }
}

