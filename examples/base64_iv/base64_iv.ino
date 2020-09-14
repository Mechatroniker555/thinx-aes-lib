/* Example taking test values from node.js server as input as well... */

#include <AESLib.h>

AESLib aesLib;

String plaintext = "12345678;";
int loopcount = 0;

char cleartext[256];
char ciphertext[512];

// AES Encryption Key
byte aes_key[] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };

// General initialization vector (you must use your own IV's in production for full security!!!)
byte aes_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// Sample strings as generated by node.js server
String server_b64iv = "AAAAAAAAAAAAAAAAAAAAAAAA=="; // same as aes_iv  but in Base-64 form as received from server
String server_b64msg = "j0RFVdlKjYrwx17qzHdt40ZS4hxckx0riP4SNy21X3U="; // CBC/Zeropadding; same as aes_iv  but in Base-64 form as received from server

byte * nullIV() {
  byte null_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // iv_block gets written to, provide own fresh copy...
  return null_iv;
}

// Generate IV (once)
void aes_init() {

  Serial.flush();
  
  delay(1000);

  Serial.println("\n=======\n");

  ///
  
  Serial.println("\n1) AES init... paddingMode::ZeroLength");  
  aesLib.set_paddingmode(paddingMode::ZeroLength);  

  byte enc_iv_A[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  Serial.println("Encrypting \"Looks like key but it's not me.\" using null-IV with ZeroLength padding");
  String encrypted1 = encrypt( (const char*) "Looks like key but it's not me.\0", enc_iv_A );
  Serial.print("Encrypted(1): "); Serial.println(encrypted1);
  print_key_iv();

  aesLib.set_paddingmode(paddingMode::ZeroLength);  

  byte dec_iv_B[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  Serial.println("Decrypting \"Looks like key but it's not me.\" using null-IV ZeroLength padding");
  String decrypted = decrypt((char*)encrypted1.c_str(), dec_iv_B); // aes_iv fails here, incorrectly decoded...
  Serial.print("Cleartext: ");
  Serial.println(decrypted);
  Serial.println("In first iteration this should work (using untouched dec_iv_B) ^^^");

  ///
  
  Serial.println("\n2) AES init... paddingMode::ZeroLength");
  aesLib.set_paddingmode(paddingMode::ZeroLength);  

  Serial.println("Encrypting \"Looks like key but it's not me.\" using null-IV with ZeroLength padding");
  String encrypted2 = encrypt( (const char*) "Looks like key but it's not me.", nullIV() );
  Serial.print("Encrypted (2): "); Serial.println(encrypted2);
  print_key_iv();

  aesLib.set_paddingmode(paddingMode::ZeroLength);

  Serial.println("Decrypting \"Looks like key but it's not me.\" using null-IV ZeroLength padding");
  decrypted = decrypt((char*)encrypted2.c_str(), nullIV()); // aes_iv fails here, incorrectly decoded...
  Serial.print("Cleartext: ");
  Serial.println(decrypted);
  Serial.println("In second iteration this fails unless using proper null_iv ^^^");
  
  ///

  Serial.println("\n3) AES init... from Server, paddingMode::ZeroLength");
  aesLib.set_paddingmode(paddingMode::ZeroLength);  

  int ivLen = base64_decode((char*)server_b64iv.c_str(), (char *)aes_iv, server_b64iv.length());
  Serial.print("Server IV should be null-IV: ");
  print_key_iv();
  Serial.print("Decoded Server IV bytes to aes_iv: "); Serial.println(ivLen);

  Serial.print("Ciphertext: "); Serial.println(server_b64msg);
  byte dec_iv_C[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  int msgLen = strlen(server_b64msg.c_str());
  char xdecrypted[msgLen]; // half may be enough
  aesLib.decrypt((unsigned char*)server_b64msg.c_str(), msgLen, xdecrypted, aes_key, sizeof(aes_key), dec_iv_C);
  decrypted=String(xdecrypted);

  Serial.print("Server message decrypted using server IV and ZeroLength, cleartext: ");
  Serial.println(decrypted);
  
  // reset aes_iv to server-based value
  ivLen = base64_decode((char*)server_b64iv.c_str(), (char *)aes_iv, server_b64iv.length());
  // Serial.print("Server IV should be null-IV now: ");¨
  // array/mem copy first!  
  print_key_iv();
  Serial.print("Decoded Server IV bytes: "); Serial.println(ivLen);  

  ///

  Serial.println("\n4) AES init... for Server, paddingMode::ZeroLength");
  aesLib.set_paddingmode(paddingMode::ZeroLength);  

  byte msg[] = "Looks like key but it's not me.";
  byte iv_D[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  int outLen = strlen((const char*)msg);
  char encrypted[2 * msgLen];
  int enclen = aesLib.encrypt((const unsigned char*)msg, outLen, encrypted, aes_key, sizeof(aes_key), iv_D);
  encrypted[enclen] = 0; // terminate string explicitly

  Serial.print("Ciphertext: "); Serial.println(encrypted);
  Serial.print("enclen: "); Serial.println(enclen);

  Serial.println("\n---\n");
   
  // reset aes_iv to server-based value
  ivLen = base64_decode((char*)server_b64iv.c_str(), (char *)aes_iv, server_b64iv.length());
  // Serial.print("Server IV should be null-IV now: ");¨
  // array/mem copy first!  
  print_key_iv();
  Serial.print("Decoded Server IV bytes: "); Serial.println(ivLen);  

  
}

String encrypt(char * msg, byte iv[]) {
  int msgLen = strlen(msg);
  char encrypted[2 * msgLen];
  aesLib.encrypt64(msg, msgLen, encrypted, aes_key, sizeof(aes_key), iv);
  return String(encrypted);
}

String decrypt(char * msg, byte iv[]) {
  int msgLen = strlen(msg);
  char decrypted[msgLen]; // half may be enough
  aesLib.decrypt64(msg, msgLen, decrypted, aes_key, sizeof(aes_key), iv);
  return String(decrypted);
}

void print_key_iv() {
  Serial.print("AES IV: ");
  for (unsigned int i = 0; i < sizeof(aes_iv); i++) {
    Serial.print(aes_iv[i], DEC);
    if ((i + 1) < sizeof(aes_iv)) {
      Serial.print(",");
    }
  }

  Serial.println("");
}

void setup() {
  Serial.begin(9600);
  aes_init();
}

void loop() {

  loopcount++;

  if (loopcount > 5) return; // prevent week-long logs

  //sprintf(cleartext, "START; %i \n", loopcount);
  sprintf(cleartext, "Looks like key but it's not me.", loopcount);

  aesLib.set_paddingmode(paddingMode::ZeroLength);

  // Encrypt Data
  byte enc_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // iv_block gets written to, provide own fresh copy...
  String encrypted = encrypt(cleartext, enc_iv);
  sprintf(ciphertext, "%s", encrypted.c_str());
  Serial.print("Base64 encoded Ciphertext: ");
  Serial.println(encrypted);

  // Decrypt Data
  byte dec_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // iv_block gets written to, provide own fresh copy...
  String decrypted = decrypt(ciphertext, dec_iv);
  Serial.print("Base64 encoded Cleartext: ");
  Serial.println(decrypted);

  delay(500);

}
