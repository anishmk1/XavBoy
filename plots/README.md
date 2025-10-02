Performance Investigation Findings:

From instrumenting the code with timers, I found that SDL event loop was by far the biggest bucket, even though its only function was to poll for X clicking. Although all user input will eventually use this.
On researching, found that general practice is to poll for sdl events once per frame. I was doing it once per every main loop iteration - which is 1000s of times.
Changing this resulted in performance_pie_chart-3.png. SDL events is completely wiped out from the frame timings. It is negligible now

Still need to account for 16.8% of avg frame time.
