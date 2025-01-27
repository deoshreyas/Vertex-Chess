<img src="https://raw.githubusercontent.com/deoshreyas/Vertex-Chess/refs/heads/main/Vertex-Logo.png" alt="Vertex Logo" width=500>

# Vertex-Chess
A strong chess engine made using the C++ programming language! It is strong enough to beat me, with a decent (I hope!) evaluation function. This is my second attempt at making an engine, as the first one was riddled with bugs. This time, I properly tested each new feature by having the engine play 1000 games against the previous version (100ms per move). So, every single feature in the engine is there after a LOT of testing!

## :question: How to run?
Currently, the engine does not support the Universal Chess Interface (UCI) protocol, so it won't work with traditional Chess GUIs. However, there are still several ways to play with it:

1. **Web GUI _(recommended for most users)_:** 

You can use the Web GUI [here](https://deoshreyas.github.io/Vertex-Chess/)! I ported the C/C++ code to Web Assembly, and used Web Workers to make this! It supports four time controls - Bullet (1 min | 5 sec), Blitz (5 min | 10 sec), Rapid (10 min | 15 sec) and Classical (30 min)! You can also set a predetermined time to spend per move for the engine (**highly recommended** - I have very little faith in my ability to code proper time management!). If you are feeling brave, you can set it to 0 and play!

<div style="text-align: center">
    <img src="https://raw.githubusercontent.com/deoshreyas/Vertex-Chess/refs/heads/main/Demo/WebUI_Demo.gif" width="50%">
</div>

2. **Python GUI _(recommended for people looking for more control)_:**

You can also use the Python GUI, created using Pygame and Python-Chess by cloning the [GUI folder](https://github.com/deoshreyas/Vertex-Chess/tree/main/GUI). This is recommended if you want to play a custom position, with custom time controls, etcetera. I had to port the C++ (and a bit of C) code to a Dynamic Link Library to make this, to use it from within Python. 

Once you have cloned the Repository, you might have to install some packages like `pygame` and `chess` using the command:
```
pip install pygame
```
You can do this in a Virtual Environment if you want! The GUI folder comes with a precompiled DLL already, so you don't have to do it yourself!

<div style="text-align: center">
    <img src="https://raw.githubusercontent.com/deoshreyas/Vertex-Chess/refs/heads/main/Demo/PythonGUI_Demo.gif" width="50%">
</div>

3 **Build from source:**

If you want to, you can download the [source code](https://github.com/deoshreyas/Vertex-Chess/tree/main/src) and compile it yourself. Please feel free to change it to suit your own needs!

## :star: Gratitude:

I am deeply grateful to the following individuals:
1. [Disservin](https://github.com/Disservin/chess-library) for the Chess Library which handles move generation, Bitboards, etcetera!
2. The creators of the `komodo.bin` opening book which all versions of Vertex currently use!
3. The people behind the [Chess Programming Wiki](https://www.chessprogramming.org/)! This is a very extensive resource for learning more about chess engines - their search algorithms, evaluation functions, ectera! Very highly recommended for beginners. 