#include <Fuzzy.h>

// cam
#include <SPI.h>
#include <Pixy2.h>
Pixy2 pixy;

////////////////////////////////////////////////////////

// ENA IN1 IN2 IN3 IN4 ENB
#define IN1 4 // deklarasi pin IN1
#define IN2 5  // deklarasi pin IN2
#define IN3 6  // deklarasi pin IN3
#define IN4 7  // deklarasi pin IN4
#define ENA 8 // deklarasi pin ENA
#define ENB 9  // deklarasi pin ENB

// deadzone kanan kiri
//float deadZone = 0.15;

////////////////////////////////////////////////////////

int cont = 0;
int signature, x, y, width, height;
float cx, cy, area;

// Fuzzy
Fuzzy *fuzzy = new Fuzzy();

// FuzzyInput 1 - CH1 Maju
FuzzySet *Kiri = new FuzzySet(-1, -1, -0.5, -0.45);
FuzzySet *Tengah = new FuzzySet(-0.5, -0.45, 0.45, 0.5);
FuzzySet *Kanan = new FuzzySet(0.45, 0.5, 1, 1);

// FuzzyInput 2 - Sensor depan
FuzzySet *Jauh = new FuzzySet(0, 0, 2000, 3500);
FuzzySet *Sedang = new FuzzySet(2000, 3500, 5500, 8000);
FuzzySet *Dekat = new FuzzySet(5500, 8000, 30000, 30000);

// FuzzyOutput 1 - Roda Kiri
FuzzySet *vl_Slambat = new FuzzySet(0, 0, 25 , 45);
FuzzySet *vl_lambat = new FuzzySet(25, 45, 50 , 60);
FuzzySet *vl_cepat = new FuzzySet(50, 60, 80 , 80);

// FuzzyOutput 2 - Roda Kanan
FuzzySet *vr_Slambat = new FuzzySet(0, 0, 25 , 45);
FuzzySet *vr_lambat = new FuzzySet(25, 45, 50 , 60);
FuzzySet *vr_cepat = new FuzzySet(50, 60 , 75 , 75);

void setup()
{
  // Set the Serial output
  Serial.begin(115200);
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  Serial.print("Starting...n");
  pixy.init();
  // Set a random seed
  randomSeed(analogRead(0));

  // Every setup must occur in the function setup()

  // FuzzyInput
  FuzzyInput *sensorposisi  = new FuzzyInput(1);

  sensorposisi->addFuzzySet(Kiri);
  sensorposisi->addFuzzySet(Tengah);
  sensorposisi->addFuzzySet(Kanan);
  fuzzy->addFuzzyInput(sensorposisi);

  // FuzzyInput
  FuzzyInput *sensorluas  = new FuzzyInput(2);

  sensorluas->addFuzzySet(Dekat);
  sensorluas->addFuzzySet(Sedang);
  sensorluas->addFuzzySet(Jauh);
  fuzzy->addFuzzyInput(sensorluas);


  // FuzzyOutput
  FuzzyOutput *rodakanan = new FuzzyOutput(1);

  rodakanan->addFuzzySet(vr_Slambat);
  rodakanan->addFuzzySet(vr_lambat);
  rodakanan->addFuzzySet(vr_cepat);
  fuzzy->addFuzzyOutput(rodakanan);

  // FuzzyOutput
  FuzzyOutput *rodakiri = new FuzzyOutput(2);

  rodakiri->addFuzzySet(vl_Slambat);
  rodakiri->addFuzzySet(vl_lambat);
  rodakiri->addFuzzySet(vl_cepat);
  fuzzy->addFuzzyOutput(rodakiri);

  //Building FuzzyRule 1
  FuzzyRuleAntecedent *KiAndDe = new FuzzyRuleAntecedent();
  KiAndDe->joinWithAND(Kiri, Dekat);
  
  FuzzyRuleConsequent *O1_kiri = new FuzzyRuleConsequent();
  O1_kiri->addOutput(vr_lambat);
  O1_kiri->addOutput(vl_Slambat);

  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, KiAndDe, O1_kiri);
  fuzzy->addFuzzyRule(fuzzyRule1);
  
//rule 2
  FuzzyRuleAntecedent *kiAndSe = new FuzzyRuleAntecedent();
  kiAndSe->joinWithAND(Kiri, Sedang);
  
  FuzzyRuleConsequent *O2_kiri = new FuzzyRuleConsequent();
  O2_kiri->addOutput(vr_cepat);
  O2_kiri->addOutput(vl_Slambat);

  FuzzyRule *fuzzyRule2 = new FuzzyRule(2, kiAndSe, O2_kiri);
  fuzzy->addFuzzyRule(fuzzyRule2);
  
//rule 3
  FuzzyRuleAntecedent *kiAndja = new FuzzyRuleAntecedent();
  kiAndja->joinWithAND(Kiri, Jauh);
  
  FuzzyRuleConsequent *O3_kiri = new FuzzyRuleConsequent();
  O3_kiri->addOutput(vr_cepat);
  O3_kiri->addOutput(vl_lambat);

  FuzzyRule *fuzzyRule3 = new FuzzyRule(3, kiAndja, O3_kiri);
  fuzzy->addFuzzyRule(fuzzyRule3);

//rule 4
  FuzzyRuleAntecedent *TeAndDe = new FuzzyRuleAntecedent();
  TeAndDe->joinWithAND(Tengah, Dekat);
  
  FuzzyRuleConsequent *O4_lurus = new FuzzyRuleConsequent();
  O4_lurus->addOutput(vr_Slambat);
  O4_lurus->addOutput(vl_Slambat);

  FuzzyRule *fuzzyRule4 = new FuzzyRule(4, TeAndDe, O4_lurus);
  fuzzy->addFuzzyRule(fuzzyRule4);
  
  //rule 5
  FuzzyRuleAntecedent *TeAndSe = new FuzzyRuleAntecedent();
  TeAndSe->joinWithAND(Tengah, Sedang);
  
  FuzzyRuleConsequent *O5_lurus = new FuzzyRuleConsequent();
  O5_lurus->addOutput(vr_lambat);
  O5_lurus->addOutput(vl_lambat);

  FuzzyRule *fuzzyRule5 = new FuzzyRule(5, TeAndSe, O5_lurus);
  fuzzy->addFuzzyRule(fuzzyRule5);

  //rule 6
  FuzzyRuleAntecedent *TeAndJa = new FuzzyRuleAntecedent();
  TeAndJa->joinWithAND(Tengah, Jauh);
  
  FuzzyRuleConsequent *O6_lurus = new FuzzyRuleConsequent();
  O6_lurus->addOutput(vr_cepat);
  O6_lurus->addOutput(vl_cepat);

  FuzzyRule *fuzzyRule6 = new FuzzyRule(6, TeAndJa, O6_lurus);
  fuzzy->addFuzzyRule(fuzzyRule6);

  //rule 7
  FuzzyRuleAntecedent *KaAndDe = new FuzzyRuleAntecedent();
  KaAndDe->joinWithAND(Kanan, Dekat);
  
  FuzzyRuleConsequent *O7_kanan = new FuzzyRuleConsequent();
  O7_kanan->addOutput(vr_Slambat);
  O7_kanan->addOutput(vl_lambat);

  FuzzyRule *fuzzyRule7 = new FuzzyRule(7, KaAndDe, O7_kanan);
  fuzzy->addFuzzyRule(fuzzyRule7);

  //rule 8
  FuzzyRuleAntecedent *KaAndSe = new FuzzyRuleAntecedent();
  KaAndSe->joinWithAND(Kanan, Sedang);
  
  FuzzyRuleConsequent *O8_kanan = new FuzzyRuleConsequent();
  O8_kanan->addOutput(vr_Slambat);
  O8_kanan->addOutput(vl_cepat);

  FuzzyRule *fuzzyRule8 = new FuzzyRule(8, KaAndSe, O8_kanan);
  fuzzy->addFuzzyRule(fuzzyRule8);

  //rule 9
  FuzzyRuleAntecedent *KaAndJa = new FuzzyRuleAntecedent();
  KaAndJa->joinWithAND(Kanan, Jauh);
  
  FuzzyRuleConsequent *O9_kanan = new FuzzyRuleConsequent();
  O9_kanan->addOutput(vr_lambat);
  O9_kanan->addOutput(vl_cepat);

  FuzzyRule *fuzzyRule9 = new FuzzyRule(9, KaAndJa, O9_kanan);
  fuzzy->addFuzzyRule(fuzzyRule9);


}

void loop()
{
/*Serial.print("PWM A : ");
           Serial.println(ENA);
           Serial.print("PWM B : ");
           Serial.println(ENB);*/
 // float input1 = random(-0.96, 0.88);
 // float input1 = -0.67;
 // int input2 = 5800;
 float turn = pixyCheck();
 float luas = area;
 
 

  Serial.println("\n\n\nEntrance: ");
  Serial.print("\t\t\tPosisi: ");
  Serial.print(turn);
  Serial.print(", Luas area: ");
  Serial.println(luas);



 fuzzy->setInput(1, turn);
 fuzzy->setInput(2, luas);


 fuzzy->fuzzify();

 float output_vr = fuzzy->defuzzify(1);
 float output_vl = fuzzy->defuzzify(2);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, output_vr); // Mengatur kecepatan motor A (0-255)
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, output_vl); // Mengatur kecepatan motor B (0-255)

 // roda_kanan.setSpeed(output_vr);
 // roda_kiri.setSpeed(output_vl);

  
  Serial.println("Result: ");
  Serial.print("\t\t\t Vr: ");
  Serial.print(output_vr);
  Serial.print(", and Vl: ");
  Serial.println(output_vl);

  delay(100);
  
}
float pixyCheck() {
  static int i = 0;
  int j;
  uint16_t blocks;
  char buf[32];
  // grab blocks!
  blocks = pixy.ccc.getBlocks();

  // If there are detect blocks, print them!
  if (blocks)
  {
    signature = pixy.ccc.blocks[0].m_signature;
    height = pixy.ccc.blocks[0].m_height;
    width = pixy.ccc.blocks[0].m_width;
    x = pixy.ccc.blocks[0].m_x;
    y = pixy.ccc.blocks[0].m_y;
    cx = (x + (width / 2));
    cy = (y + (height / 2));
    cx = mapfloat(cx, 0, 320, -1, 1);
    cy = mapfloat(cy, 0, 200, 1, -1);
    area = width * height;

            Serial.print("sig: ");
            Serial.print(signature);
            Serial.print(" x:");
           Serial.print(x);
           Serial.print(" y:");
           Serial.print(y);
           
    //        Serial.print(" width: ");
    //        Serial.print(width);
    //        Serial.print(" height: ");
    //        Serial.print(height);
    //        Serial.print(" cx: ");
    //        Serial.print(cx);
    //        Serial.print(" cy: ");
    //        Serial.println(cy);

  }
  else {
    cont += 1;
    if (cont == 100) {
      cont = 0;
      cx = 0;
    }
  }
  return cx;
}

float mapfloat(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}
