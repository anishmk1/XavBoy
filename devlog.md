Dec 5th 2025:

Issue: After Nintendo boot logo, only seeing blank white screen instead of credits

Fixes:
1. Missed implementing for OAM-DMA transfer
2. After the OAM-DMA transfer, Tetris code does some other setup and then enables LCD with 0xd3. After this, it checks for "all buttons pressed". This is considered a soft reset allowed by the game. My Joypad register resets to 0xCF (the lower nibble = F indicates all buttons are not pressed). However I missed that these bits should be read-only. So when the game tries to write to this register to update the "select bits", the lower nibble should not be modified. Instead, I was allowing the game to write 0 to the lower nibble and so Tetris always considered this as a soft reset.
3. My VBLANK interrupt request happened at the start of every scanline as long as it was VBLANK mode. It should only happen once at LY=144. This was causing the interrupt handler to trigger every scanline and it would reset all the setup progress made previously.

Next: Looks like release build is not displaying the credit screen. Only the step compilation does on frame 3. Probably undefined behavior somewhere.