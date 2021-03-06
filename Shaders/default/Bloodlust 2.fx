//Author: Scanti
//Gives a red ghosting effect

texture lastframe;
texture lastpass;

technique T0
{
   Pass P0
   {
       Texture[1] = <lastframe>;
       PixelShader = asm
       {
           ps.1.3             //A modification of Timeslip's motion only
           def c0,1,0,0,1     //code to get a red ghosting effect.
           tex t0  
           tex t1  
           sub_sat r0,t0,t1
           sub_sat r1,t1,t0
           add r0.rgb,r0,r1
           mul r0,r0,c0
           add r0,r0,t0
           mov r0.a,c0  
       };
   }

   Pass P1
   {
       texture[1]=<lastpass>;
 
       PixelShader = asm
       {
           ps.1.2

           def c0,0.80,0.80,0,1
           def c1,0.80,0.80,0.80,1
           def c2,0,1,1,1
     
           texcoord t0
           tex t1
           mov r0,t0_bx2
           mul r0,r0,r0
           dp3_sat r0,r0,c0
           sub r0,r0,c1
           add_sat r0,r0,c2
           mul r0,t1,1-r0
           mov r0.a,c0
       };
   }
}
