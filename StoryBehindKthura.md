# Story behind Kthura

Kthura is an object oriented 2D map editor, originally meant for creating the point-and-click adventure "Mörker". Kthura is named after that game's main protagonist, as such her face became the logo. Unfortunately, I had to cancel that project in an early stage, due to my limited skills in creating artwork and Mörker did require too much unique graphics to do properly.

As such, Kthura as a map editor was a bit fated to a pre-mature death, as I didn't yet have much other plans, however, when I began the Star Story project, I just came up with the incredibly retarded idea, really, I still don't know what I was thinking to try if Kthura could be suitable for RPG map design, and the rest is a kind of history... well from my point of view it is.

The first version of Kthura was written in BlitzMax, however as the BlitzMax compiler refuses to work on me (and Brucey's BlitzMax Next Generation didn't work for me either), I was forced to rewrite Kthura in a different language. At first that became C# with the help of MonoGame. However MonoGame took the stupid idea to be completely dependent on NuGet and I simply hated that. It would make me more dependent on their site that I need to be, and I prefer to work on my own (Kthura was designed for a reason).

So I took on the crazy path to learn C++. 
Now to tell the truth, Kthura in C++ was a total disaster. In both BlitzMax and C# Kthura simply was dead easy to create due to its very simplistic approach to memory control... As a matter of fact, that is completely out of my hands. And both languages are just OOP-paradise, which Kthura very heavily relies on. C++ is officially OOP due to class support, but to be frank, the memory control is manually, and my worst issue is the lack of a proper garbage collector.
Now extended classes didn't turn out too well either, well, what did you expect, over 25 or 30 years of coding experience in general, but in C++ I was once again a newbie. I tried to save things with shared pointers, which did at least make sure everything worked. But I also feared those shared pointers slowed stuff down, and also those things came later as a bit of a try to save the project. 

All in all, the original C++ libraries for Kthura can be seen as just some crap dumped together by an inexperienced C++ user who had just grown too comfortable with the luxuries C#, BlitzMax and other high-level languages provided.

Now Kthura was not the only library suffering here. JCR6 (which Kthura also heavily relies on) and some other things I made for C++ were not that much better of.

Well, nobody gets the perfect job done in their first go (if they can actually ever get the perfect job done, at all). And that's why I was thinking for months to set the Slyvina framework up.
In case you care, Slyvina is named after the protagonist of some experimental stories in which I mixed murder plots with fantasy, in which Slyvina is the name of the one who has to sort out who actually comitted the murder.

And I was determined, if Slyvina would ever come to be, Kthura will be completely redone for it.
And well, welcome to the repository where this new Kthura will live on.

- Now maps designed with older Kthura versions should still be working
- Case sensitivity in TagMaps and LabelMaps was deprecated on the moment I began the original C++ Kthura library. Needless to say that It'll be removed completely now. So both maps will be case INsensitive

Oh, and the girl called Kthura did make her way into The Secrets of Dyrt. There's a sidequest tied to the MEDALS system which can open up a portal to Mörker in there. So the girl Kthura did have a life in the end. 
Perhaps if somebody (who lives closeby Breda) is interested to do Mörker with me, I can see if the project gets resumed. 
