# Arduino Glowing Pumpkin (Adafruit Feather)

Last year, I created a cool [Arduino-powered Glowing Pumpkin](https://github.com/johnwargo/Arduino-Glowing-Pumpkin) project for Halloween. I used a NeoPixel ring and an Arduino to display random colors inside a foam pumpkin. I built several of them, and placed one in each dormer window in my house. they work great. I've always wanted to do something bigger, but I never found a big enough pumpkin to use for this. This year, while picking up some supplies at Home Depot, I found the perfect pumpkin for an updated (larger) version of the project.

As you can see from the following picture, it's a pretty big pumpkin; that's a quart paint can to the right of it.

![Finished Project](glowing-pumpkin-feather/images/figure-01.png)

For lighting, I had to think bigger. Since this was a much larger pumpkin to illuminate, I had to go beyond the simple NeoPixel ring. For this project, I used the Adafruit NeoPixel FeatherWing, a 8x4 array of bright, tri-color LEDs (that's 32 LEDs for those of you who are mathematically challenged). I then paired the NeoPixel FeatherWing with the Adafruit Feather HUZZAH since I wanted to do something interesting on the network for a version of this.

So, there are two versions of the project:

+	The [Feather HUZZAH and NeoPixel FeatherWing](glowing-pumpkin-feather/readme.md) version of the project. This version simply illuminates the pumpkin with bright lights, cycling through multiple colors over time.
+ The [Feather HUZZAH, NeoPixel FeatherWing, and Adalogger FeatherWing](glowing-pumpkin-feather-timer/readme.md) version. This version of the project does the same things as the other versions, but for this version, I used a real-time clock FeatherWing and the Feather's Wi-Fi network connection to connect to a time server, update the board's clock with the correct time, then turn the NeoPixels on and off on a predetermined schedule (set by you in the project's code). How cool is that? 

Here's a [video of the final project in action](https://vimeo.com/236226091).

***

If you find this code useful, and feel like thanking me for providing it, please consider making a purchase from [my Amazon Wish List](https://amzn.com/w/1WI6AAUKPT5P9). You can find information on many different topics on my [personal blog](http://www.johnwargo.com). Learn about all of my publications at [John Wargo Books](http://www.johnwargobooks.com).