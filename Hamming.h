#ifndef Hamming_h
#define Hamming_h

#include <Arduino.h>
#include <assert.h>
#define maxOriginalMessageSize 32

class Hamming
{
public:
    Hamming(byte messageLength);
    Hamming();
    void Init(byte messageLength)
    {
        initialize(messageLength);
    }

    //encoded methods
    void encode(byte originalMessage[], byte encodedMessage[]);

    //parity check methods
    bool parityCheckAndErrorCorrection(byte encodedMessage[]);
    void parityCheckAndErrorCorrectionMulti(byte encodedMessage[], int lengthOfBitsIn);

    //decode methods
    //void decodeMessageToASCII(byte encodedMessage[]);
    // void decodeMessageToASCIIMulti(byte encodedMessage[], int lengthOfBitsIn);

    //get methods
    int getMessageLength();
    int getEncodedMessageLength();
    int getParityBitsLength();

    //visualize G or H matrix
    void printGMatrix();
    void printHMatrix();

private:
    //constructor or init methods
    void initialize(byte messageLength);
    void setUpEncodedMessageLengthAndParityBits();
    void createGeneratorAndParityCheckMatrix();
    void setUpErrorDictionary();

    //parity check methods
    int getBitIndexOfError(int syndromeInt);
    int getSyndromeVectorAsInteger(byte encodedMessage[]);

    //visualize incoming matrix
    void print2DArray(byte arr[][maxOriginalMessageSize + 10], byte rows, byte cols);

    byte messageLength;
    byte parityBits;
    byte encodedMessageLength;
    byte g[maxOriginalMessageSize + 10][maxOriginalMessageSize + 10];
    byte h[10][maxOriginalMessageSize + 10];
    int errorDictionary[maxOriginalMessageSize];
};

#endif