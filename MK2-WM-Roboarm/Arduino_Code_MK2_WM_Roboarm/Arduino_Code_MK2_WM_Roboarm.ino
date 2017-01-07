#include "MyTypes.h"
#define stpmode 32 // Sets the stepping mode of all 3 motors. Set to 1, 2, 4, 8, 16, or 32

// Set pin numbers for steppers A, B, and C
const int astep = 4;
const int adir = 26;
const int bstep = 5;
const int bdir = 27;
const int cstep = 6;
const int cdir = 28;

// Set pin numbers for stepping mode selection
const int stpmode0 = 32;
const int stpmode1 = 33;
const int stpmode2 = 34;

// Cartesian coordinate calculation variables (mm)
const int xoffset = 40;
const int yoffset = 27;

const int startbutton = A2;

// Set lengths of humerus and ulna
float h = 249.2;
float u = 249.2;



// Create arrays used to store the position of each predefined waypoint {xcoord, ycoord, leftorrightbend, baseangle}
waypoint homepos = {0, h + u, 1, 90}; 
waypoint stack1 = {-280, -213, 0, 0};
waypoint stack2 = {85, 68, 1, 0};
waypoint stack3 = {295, 277, 0, 0};
waypoint stack4 = {85, 487, 1, 0};
waypoint stack5 = {-124, 277, 0, 0};
waypoint targetcenter = {85, 277, 0, 0};

waypoint wp[7] = {homepos, stack1, stack2, stack3, stack4, stack5, targetcenter};

// Array for storing calculated angles
float angles[2];

// Number of loops through main loop
int k = 1;

long astepcount = adegreesstep(90);
long bstepcount = bdegreesstep(0);
long cstepcount = cdegreesstep(0);

long astepdifference = 0;
long bstepdifference = 0;
long cstepdifference = 0;

int astepdirection = 0;
int bstepdirection = 0;
int cstepdirection = 0;

int astepswitch = 0;
int bstepswitch = 0;
int cstepswitch = 0;

unsigned long astepswitchtimer = 0;
unsigned long bstepswitchtimer = 0;
unsigned long cstepswitchtimer = 0;
int effectorpositionstatus = 0;

int runonstarta = 1;

// Set ratio for number of steps to number of degrees
const float aratio = 200.0 * 32 / 10 * stpmode / 360.0;
const float bratio = 200.0 * 24 / 10 * stpmode / 360.0;
const float cratio = 200.0 * 50.0 * stpmode / 360.0;

const unsigned long motadelay = 5000 / aratio * 2;
const unsigned long motbdelay = 5000 / bratio * 1.5;
const unsigned long motcdelay = 1000 / cratio;

unsigned long currentmotadelay = motadelay;
unsigned long currentmotbdelay = motbdelay;

const unsigned long motamaxdelay = motadelay * 1000;
const unsigned long motbmaxdelay = motbdelay * 1000;

const int motascalefactor = 100;
const int motbscalefactor = 100;

int motadelayfactor = 0;
int motbdelayfactor = 0;

int waypointselect = 1;

void setup() {
  // Set each control pin to an output
  pinMode(astep, OUTPUT);
  pinMode(adir, OUTPUT);
  pinMode(bstep, OUTPUT);
  pinMode(bdir, OUTPUT);
  pinMode(cstep, OUTPUT);
  pinMode(cdir, OUTPUT);
  pinMode(stpmode0, OUTPUT);
  pinMode(stpmode1, OUTPUT);
  pinMode(stpmode2, OUTPUT);
  pinMode(startbutton, INPUT_PULLUP);

  digitalWrite(astep, LOW);
  digitalWrite(bstep, LOW);
  digitalWrite(cstep, LOW);
  
  digitalWrite(adir, LOW);
  digitalWrite(bdir, LOW);
  digitalWrite(cdir, LOW);

  // Set stepping mode based on the value of stpmode 
  
  // 1/2 microstep mode
  if(stpmode == 2) {
    digitalWrite(stpmode0, HIGH);
    digitalWrite(stpmode1, LOW);
    digitalWrite(stpmode2, LOW);
  }
  // 1/4 microstep mode
  else if(stpmode == 4) {
    digitalWrite(stpmode0, LOW);
    digitalWrite(stpmode1, HIGH);
    digitalWrite(stpmode2, LOW);
  }
  // 1/8 microstep mode
  else if(stpmode == 8) {
    digitalWrite(stpmode0, HIGH);
    digitalWrite(stpmode1, HIGH);
    digitalWrite(stpmode2, LOW);
  }
  // 1/16 microstep mode
  else if(stpmode == 16) {
    digitalWrite(stpmode0, LOW);
    digitalWrite(stpmode1, LOW);
    digitalWrite(stpmode2, HIGH);
  }
  // 1/32 microstep mode
  else if(stpmode == 32) {
    digitalWrite(stpmode0, HIGH);
    digitalWrite(stpmode1, LOW);
    digitalWrite(stpmode2, HIGH);
  }
  // Full step mode
  else {
    digitalWrite(stpmode0, LOW);
    digitalWrite(stpmode1, LOW);
    digitalWrite(stpmode2, LOW);
  }

  Serial.begin(9600);
  
  //astepdifference=50*stpmode;
  //bstepdifference=50*stpmode*3/2;
  //cstepdifference=50*stpmode*50;
  
  
}

void loop() {
  long stepdifferenceall = astepdifference + bstepdifference + cstepdifference;
 
  if (stepdifferenceall == 0 && waypointselect < 7) {
    inversekinematics(wp[waypointselect]);
    waypointselect++;
    delay(500);
  }
    
  
  switch (astepswitch) {
    case 0:
      if ((astepdifference != 0) && (cstepdifference == 0)) {
        astepswitch = 1;
        astepswitchtimer = micros();
        currentmotadelay = motamaxdelay;
        motadelayfactor = 0;
      }
      break;
    case 1:
      if(micros() >= (astepswitchtimer + currentmotadelay)) {
        digitalWrite(astep, LOW);
        astepswitchtimer = micros();
        astepswitch = 2;
      }
      break;
    case 2:
      if(micros() >= (astepswitchtimer + currentmotadelay)) {
        digitalWrite(astep, HIGH);
        astepswitchtimer = micros();
        if(astepdirection == 0) {
          astepcount++;
          
        }
        else if(astepdirection == 1) {
          astepcount--;
        }
        astepdifference--;
        if(astepdifference == 0) {
          astepswitch = 0;
        }
        else {
          astepswitch = 1;
        }
        
        if(motadelayfactor <= motascalefactor) {
          motadelayfactor++;
          currentmotadelay = ((pow(motadelayfactor - motascalefactor, 2) / 4) + motadelay);
        }
      }
      break;
  }


  switch (bstepswitch) {
    case 0:
      if ((bstepdifference != 0) && (cstepdifference == 0)) {
        bstepswitch = 1;
        bstepswitchtimer = micros();  
        currentmotbdelay = motbmaxdelay;
        motbdelayfactor = 0;
      }
      break;
    case 1:
      if(micros() >= (bstepswitchtimer + currentmotbdelay)) {
        digitalWrite(bstep, LOW);
        bstepswitchtimer = micros();
        bstepswitch = 2;
      }
      break;
    case 2:
      if(micros() >= (bstepswitchtimer + currentmotbdelay)) {
        digitalWrite(bstep, HIGH);
        bstepswitchtimer = micros();
        if(bstepdirection == 0) {
          bstepcount++;
        }
        else if(bstepdirection == 1) {
          bstepcount--;
        }
        bstepdifference--;
        if(bstepdifference == 0) {
          bstepswitch = 0;
        }
        else {
          bstepswitch = 1;
        }
        if(motbdelayfactor <= motbscalefactor) {
          motbdelayfactor++;
          currentmotbdelay = ((pow(motbdelayfactor - motbscalefactor, 2) / 4) + motbdelay);
        }
      }
      break;
  }

  switch (cstepswitch) {
    case 0:
      if (cstepdifference != 0) {
        cstepswitch = 1;
        cstepswitchtimer = micros();
      }
      break;
    case 1:
      if(micros() >= (cstepswitchtimer + motcdelay)) {
        digitalWrite(cstep, LOW);
        cstepswitchtimer = micros();
        cstepswitch = 2;
      }
      break;
    case 2:
      if(micros() >= (cstepswitchtimer + motcdelay)) {
        digitalWrite(cstep, HIGH);
        cstepswitchtimer = micros();
        if(cstepdirection == 0) {
          cstepcount--;
        }
        else if(cstepdirection == 1) {
          cstepcount++;
        }
        cstepdifference--;
        if(cstepdifference == 0) {
          cstepswitch = 0;
        }
        else {
          cstepswitch = 1;
        }
      }
      break;
  }
  
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// INVERSE KINEMATICS //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

void inversekinematics(waypoint target) {
  //Calculating the two different possibilities for the beta angle
  double beta1 = atan2(  (sqrt(1-pow(((pow(target.x, 2)+pow(target.y, 2)-pow(h, 2)-pow(u, 2))/(2.0 * h * u)), 2))),((pow(target.x, 2)+pow(target.y, 2)-pow(h, 2)-pow(u, 2))/(2.0 * h * u)));
  double beta2 = atan2((-(sqrt(1-pow(((pow(target.x, 2)+pow(target.y, 2)-pow(h, 2)-pow(u, 2))/(2.0 * h * u)), 2)))),((pow(target.x, 2)+pow(target.y, 2)-pow(h, 2)-pow(u, 2))/(2.0 * h * u)));
  
  Serial.print("beta1 = ");
  Serial.println(beta1);
  Serial.print("beta2 = ");
  Serial.println(beta2);

  // Calculating the two different possibilities for the alpha angle
  float k2a = u * sin(beta1);
  float k2b = u * sin(beta2);
  float k1a = h + (u * cos(beta1));
  float k1b = h + (u * cos(beta2));
  float alpha1 = atan2(target.y, target.x)-atan2(k2a, k1a);
  float alpha2 = atan2(target.y, target.x)-atan2(k2b, k1b);

  // Create an array of the 4 calculated alpha and beta angle, converted to degrees.
  float trigresults[] = {alpha1*180.0/M_PI, alpha2*180.0/M_PI, beta1*180.0/M_PI, beta2*180.0/M_PI};

  // Make all negative angles positive
  while(trigresults[0] < 0) {
    trigresults[0] += 360;
  }
  while(trigresults[1] < 0) {
    trigresults[1] += 360;
  }
  while(trigresults[2] < 0) {
    trigresults[2] += 360;
  }
  while(trigresults[3] < 0) {
    trigresults[3] += 360;
  }
  
  Serial.println("NEWWWWWWW");
  Serial.println(trigresults[0]);
  Serial.println(trigresults[1]);
  Serial.println(trigresults[2]);
  Serial.println(trigresults[3]);

  // Variables for check that ensures angles are within permissible range
  boolean checkone = true;
  boolean checktwo = true;

  // Test alpha1 and beta1 to see if they are inside the allowable limits
  if((trigresults[0] > 200 && trigresults[0] < 360) || (trigresults[2] > 160 && trigresults[2] < 185) || (trigresults[0] != trigresults[0]) || (trigresults[2] != trigresults[2])) {
    checkone = false;
  }
  
  // Test alpha2 and beta2 to see if they are inside the allowable limits
  if((trigresults[1] > 200 && trigresults[1] < 360) || (trigresults[3] > 160 && trigresults[3] < 185) || (trigresults[1] != trigresults[1]) || (trigresults[3] != trigresults[3])) {
    checktwo = false;
  }

  // If both alpha and beta are in an acceptable range in both cases, the predefined preference will be used to determine which angles will be used.
  if(checkone && checktwo) {
    if(target.lhrh == 1) {
      angles[0] = trigresults[0];
      angles[1] = trigresults[2];
    }
    else {
      angles[0] = trigresults[1];
      angles[1] = trigresults[3];
    }
  }

  // If one of the angle sets lies in an acceptable range and the other does not, the one in range will be transferred to a new array
  else if(checkone && (!checktwo)) {
    angles[0] = trigresults[0];
    angles[1] = trigresults[2];
  }
  else if((!checkone) && checktwo) {
    angles[0] = trigresults[1];
    angles[1] = trigresults[3];
  }

  // If neither of the angle sets is in an acceptable range, the software will print an error message
  else {
    Serial.println("This point is not within the range of acceptable values for the parameters given.");
  }
  
  Serial.println(angles[0]);
  Serial.println(angles[1]);

  long stepperAtarget = adegreesstep(angles[0]);
  long stepperBtarget = bdegreesstep(angles[1]);
  long stepperCtarget = cdegreesstep(target.base);
  
  
  
  if(stepperAtarget < astepcount) {
    digitalWrite(adir, LOW);
    astepdirection = 1;
    astepdifference = astepcount - stepperAtarget; 
  }
  else if(stepperAtarget > astepcount) {
    digitalWrite(adir, HIGH);
    astepdirection = 0;
    astepdifference = stepperAtarget - astepcount;
  }
  
  if((bstepdegrees(stepperBtarget) < 180) && (bstepdegrees(bstepcount) > 180)) {
    digitalWrite(bdir, LOW);
    bstepdirection = 0;
    Serial.println("Compare Angles (2):");
    Serial.println(bstepdegrees(stepperBtarget));
    Serial.println(bstepdegrees(bstepcount));
    Serial.println("opposite1");
    bstepdifference = bdegreesstep(bstepdegrees(bdegreesstep(360) - bstepcount + stepperBtarget));
  }
  else if((bstepdegrees(stepperBtarget) > 180) && (bstepdegrees(bstepcount) < 180)) {
    digitalWrite(bdir, HIGH);
    bstepdirection = 1;
    Serial.println("Compare Angles (2):");
    Serial.println(bstepdegrees(stepperBtarget));
    Serial.println(bstepdegrees(bstepcount));
    Serial.println("opposite2");
    bstepdifference = bdegreesstep(bstepdegrees(bdegreesstep(360) - stepperBtarget + bstepcount));
  }
  else if((bstepdegrees(stepperBtarget) > bstepdegrees(bstepcount))) {
    digitalWrite(bdir, LOW);
    Serial.println("same1");
    bstepdirection = 0;
    bstepdifference = bdegreesstep(bstepdegrees(stepperBtarget - bstepcount));
  }
  else if((bstepdegrees(stepperBtarget) < bstepdegrees(bstepcount))) {
    digitalWrite(bdir, HIGH);
    Serial.println("same2");
    bstepdirection = 1;
    bstepdifference = bdegreesstep(bstepdegrees(bstepcount - stepperBtarget));
  }
  else {
    bstepdifference = 0;  
  }

  if(stepperCtarget < cstepcount) {
    digitalWrite(cdir, HIGH);
    cstepdirection = 0;
    cstepdifference = cstepcount - stepperCtarget; 
  }
  else if(stepperCtarget > cstepcount) {
    digitalWrite(cdir, LOW);
    cstepdirection = 1;
    cstepdifference = stepperCtarget - cstepcount;
  }
  else {
    cstepdifference = 0;  
  }
  Serial.print("b step difference = ");
  Serial.println(bstepdegrees(bstepdifference));
}

float astepdegrees(long steps) {
  int degreescalc = (float)steps / aratio;
  while(degreescalc > 360) {
    degreescalc -= 360.0;
  }
  while(degreescalc < 0 && degreescalc) {
    degreescalc += 360.0;
  }
  return degreescalc;
}

float bstepdegrees(long steps) {
  int degreescalc = (float)steps / bratio;
  while(degreescalc > 360) {
    degreescalc -= 360.0;
  }
  while(degreescalc < 0 && degreescalc) {
    degreescalc += 360.0;
  }
  return degreescalc;
}

float cstepdegrees(long steps) {
  int degreescalc = (float)steps / cratio;
  while(degreescalc > 360 && degreescalc) {
    degreescalc -= 360.0;
  }
  while(degreescalc < 0 && degreescalc) {
    degreescalc += 360.0;
  }
  return degreescalc;
}

long adegreesstep(float deg) {
  return (long)(deg * aratio);
}

long bdegreesstep(float deg) {
  return (long)(deg * bratio);
}

long cdegreesstep(float deg) {
  return (long)(deg * cratio);
}