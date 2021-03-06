//Author: Timeslip
//Performs an emboss based on the luminocity of each pixel instead of using each colour channel seperatey.

texture lastpass;

Technique T0
{
	Pass P0
	{
		PixelShader = asm
		{
			ps.1.4			//This is a 2 phase v1.4 pixel shader
			def c0,0.0015625,0,0,1		//Used to mod r0	(Use these value for a 640,480 resolution)
			def c1,0,0.0020833,0,1		//Used to mod r1
			def c2,0.0015625,0.0020833,0,1	//Used to mod r2
			def c3,-0.0015625,0,0,1		//Used to mod r5
			def c4,0,-0.0020833,0,1		//Used to mod r4
			def c5,-0.0015625,-0.0020833,0,1	//Used to mod r3
			//def c0,0.0009765625,0,0,1			//Used to mod r0	(Use these value for a 1024,768 resolution)
			//def c1,0,0.0013020833,0,1			//Used to mod r1
			//def c2,0.0009765625,0.0013020833,0,1	//Used to mod r2
			//def c3,-0.0009765625,0,0,1		//Used to mod r5
			//def c4,0,-0.0013020833,0,1		//Used to mod r4
			//def c5,-0.0009765625,-0.0013020833,0,1	//Used to mod r3
			def c6,0,0,0,1			//Used to set the alpha channel
			def c7,0.5,0.5,0.5,0			//Added to r0 at the end to greyify it
			texcrd r5.xyz,t0			//Get the real texcoords and stuff them in r5
			add r0.xyz,r5,c0		//Add the dx/dy to each register
			add r1.xyz,r5,c1
			add r2.xyz,r5,c2
			add r3.xyz,r5,c5
			add r4.xyz,r5,c4
			add r5.xyz,r5,c3
			phase			//Begin the second phase
			texld r0,r0		//Sample each texture
			texld r1,r1
			texld r2,r2
			texld r3,r3
			texld r4,r4
			texld r5,r5
			add r0,r0,r1		//Add or sub each texture to get a gradient
			add r0,r0,r2
			sub r0,r0,r3
			sub r0,r0,r4
			sub r0.rgb,r0,r5	//Use an rgb write mask so we can use the scaler pipe to write the alpha channel in the same
						//instruction
			+mov r0.a,c6		//Need to rewrite the alpha channel after phase is used
			add_sat r0,r0,c7	//Make the background grey instead of black
		};
	}
	Pass P1				//This is a basic greyscale function, and could be used seperately
	{
		Texture[0] = <lastpass>;	//We want to sample the results of the last pass, not the original texture
		PixelShader = asm
		{
			ps.1.4				//Performing scaler ops on the r,g and b channels, so need version 1.4
			def c0,0.33333333,0,0,1	//Use this to divide the red channel by 3 later on
			texld r0,t0			//Sample the results of the previous pass
			add r0.r,r0.r,r0.g		//Add the green channel to the red channel
			add r0.r,r0.r,r0.b		//And then the blue
			mul r0.rgb,r0.r,c0.r		//Multiply by c0 and put the result into the output register
			+mov r0.a,c0			//And at the same time fill in the alpha channel
		};
	}
}
