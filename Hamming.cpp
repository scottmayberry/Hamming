#include "Hamming.h"

Hamming::Hamming(byte messageLength)
{
    initialize(messageLength);
}
Hamming::Hamming()
{
    ;
}

void Hamming::setUpEncodedMessageLengthAndParityBits()
{
    int x = 1;
    while (pow(2, x) - x - 1 < messageLength)
    {
        x += 1;
    }
    encodedMessageLength = messageLength + x + 1;
    parityBits = x + 1;
}

void Hamming::initialize(byte messageLength)
{
    if (messageLength > maxOriginalMessageSize)
    {
        Serial.print("message size too large. needs to be less than ");
        Serial.print(maxOriginalMessageSize);
        Serial.println(". Program Aborted");
        assert(messageLength <= maxOriginalMessageSize);
    }
    this->messageLength = messageLength;
    setUpEncodedMessageLengthAndParityBits();
    createGeneratorAndParityCheckMatrix();
    setUpErrorDictionary();
}

void Hamming::createGeneratorAndParityCheckMatrix()
{
    //create parity check matrix H

    //clear parity check matrix H
    for (int j = 0; j < encodedMessageLength; j++)
    {
        for (int i = 0; i < parityBits; i++)
        {
            h[i][j] = 0;
        }
    }

    //get all numbers that are not powers of two in range of message
    int numbersToUse[messageLength];
    int numToSkip = 1;
    int ind = 0;
    int currentStep = 1;

    while (ind < messageLength)
    {
        if (numToSkip == currentStep)
        {
            numToSkip *= 2;
        }
        else
        {
            numbersToUse[ind++] = currentStep;
        }
        currentStep++;
    }

    for (int i = messageLength - 1; i >= 0; i--)
    {
        for (int j = parityBits - 2; j >= 0; j--)
        {
            h[parityBits - 1 - j][messageLength - 1 - i] = (numbersToUse[i] & (1 << j)) >> j;
        }
    }
    //set top row of 1's
    for (int i = 0; i < encodedMessageLength; i++)
    {
        h[0][i] = 1;
    }

    for (int i = 1; i < parityBits; i++)
    {
        h[i][messageLength + i] = 1;
    }

    for (int i = 1; i < parityBits; i++)
    {
        for (int j = 0; j < encodedMessageLength; j++)
        {
            if (h[i][j] == 1)
            {
                h[0][j] = byte((int(h[0][j]) - 1) * -1);
            }
        }
    }
    // printHMatrix();

    //setup Generator matrix
    //clear generator matrix
    for (int j = 0; j < encodedMessageLength; j++)
    {
        for (int i = 0; i < messageLength; i++)
        {
            g[i][j] = 0;
        }
    }
    //setup identify matrix for generator
    for (int i = 0; i < messageLength; i++)
    {
        g[i][i] = 1;
    }

    //set up -A^T matrix
    for (int i = 0; i < messageLength; i++)
    {
        for (int j = 0; j < parityBits; j++)
        {
            g[i][messageLength + j] = h[j][i];
        }
    }
    // Serial.println();
    // printGMatrix();
}

void Hamming::setUpErrorDictionary()
{
    //generate a temporary message. Use this message to generate error dictionary
    byte tempMessage[messageLength];
    for (int i = 0; i < messageLength; i++)
    {
        if (i % 2 == 0)
        {
            tempMessage[i] = 0;
        }
        else
        {
            tempMessage[i] = 1;
        }
    }

    //get the encoded message from the temp message
    byte tempEncodedMessage[encodedMessageLength];
    encode(tempMessage, tempEncodedMessage);

    //clear error dictionary for reuse
    for (int i = 0; i < maxOriginalMessageSize; i++)
    {
        errorDictionary[i] = 0;
    }

    //switch single bits of the encoded message and record the syndrome int for later use
    for (int i = 0; i < encodedMessageLength; i++)
    {
        tempEncodedMessage[i] = byte((int(tempEncodedMessage[i]) - 1) * -1);
        errorDictionary[i] = getSyndromeVectorAsInteger(tempEncodedMessage);
        tempEncodedMessage[i] = byte((int(tempEncodedMessage[i]) - 1) * -1);
    }
}

void Hamming::print2DArray(byte arr[][maxOriginalMessageSize + 10], byte rows, byte cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            Serial.print(arr[i][j]);
            Serial.print(" ");
        }
        Serial.println();
    }
}

void Hamming::encode(byte messageIn[], byte messageOut[])
{
    for (int j = 0; j < encodedMessageLength; j++)
    {
        messageOut[j] = 0;
        for (int i = 0; i < messageLength; i++)
        {
            messageOut[j] = messageOut[j] + messageIn[i] * g[i][j];
        }
        messageOut[j] = messageOut[j] % 2;
    }
}

bool Hamming::parityCheckAndErrorCorrection(byte encodedMessage[])
{
    int syndromeInt = getSyndromeVectorAsInteger(encodedMessage);
    bool noDoubleError = true;
    if (syndromeInt == 0)
    {
        return noDoubleError;
    }

    int bitIndex = getBitIndexOfError(syndromeInt);
    if (bitIndex != -1)
    {
        encodedMessage[bitIndex] = byte((int(encodedMessage[bitIndex]) - 1) * -1);
    }
    else
    {
        //Serial.print("double bit error");
        noDoubleError = false;
    }
    return noDoubleError;
}

bool Hamming::parityCheckAndErrorCorrectionNoEdit(byte encodedMessage[], byte parityCheckedMessage[])
{
    for (int i = 0; i < getEncodedMessageLength(); i++)
    {
        parityCheckedMessage[i] = encodedMessage[i];
    }
    return parityCheckAndErrorCorrection(parityCheckedMessage);
}

int Hamming::decode(byte encodedMessage[], byte decodedMessage[], int lengthOfSymbolsIn)
{
    parityCheckAndErrorCorrectionMulti(encodedMessage, lengthOfSymbolsIn);
    int decInd = 0;
    int encInd = 0;
    while (encInd < lengthOfSymbolsIn)
    {
        decodedMessage[decInd++] = encodedMessage[encInd++];
        if (decInd % getMessageLength() == 0)
        {
            encInd += getParityBitsLength();
        }
    }
    return decInd;
}

// void Hamming::decodeUntilByte(byte encodedMessage[], byte decodedMessage[])
// {
//     ;
// }

void Hamming::parityCheckAndErrorCorrectionMulti(byte encodedMessage[], int lengthOfBitsIn)
{
    int i = 0;
    byte *encodedTemp;
    while (getEncodedMessageLength() * i < lengthOfBitsIn)
    {
        encodedTemp = encodedMessage + getEncodedMessageLength() * i;
        parityCheckAndErrorCorrection(encodedTemp);
        i++;
    }
}

// int Hamming::decodeMessageToASCII(char &returnChar, byte encodedMessage[], int shiftedIndex)
// {
//     int idxFinishedOn = 0;
//     int ind = shiftedIndex;
//     int messageInd = 0;
//     int bitMultiplier = 1;
//     while (bitMultiplier != 256)
//     {
//         if (ind < getMessageLength())
//         {

//             ind++;
//         }

//         idxFinishedOn++;
//     };
//     return idxFinishedOn;
// }
// void Hamming::decodeMessageToASCIIMulti(byte encodedMessage[], int lengthOfBitsIn) { ; }

int Hamming::getBitIndexOfError(int syndromeInt)
{
    for (int i = 0; i < maxOriginalMessageSize; i++)
    {
        if (errorDictionary[i] == syndromeInt)
        {
            return i;
        }
        else
        {
            if (errorDictionary[i] == 0)
            {
                return -1;
            }
        }
    }
    return -1;
}

int Hamming::getSyndromeVectorAsInteger(byte encodedMessage[])
{
    byte tempSyndrome[parityBits];
    for (int j = 0; j < parityBits; j++)
    {
        tempSyndrome[j] = 0;
        for (int i = 0; i < encodedMessageLength; i++)
        {
            tempSyndrome[j] = tempSyndrome[j] + h[j][i] * encodedMessage[i];
        }
        tempSyndrome[j] = tempSyndrome[j] % 2;
    }
    int syndromeInt = 0;
    for (int i = parityBits - 1; i >= 0; i--)
    {
        if (tempSyndrome[i] == 1)
        {
            syndromeInt = syndromeInt + int(pow(2, parityBits - 1 - i));
        }
    }
    return syndromeInt;
}

int Hamming::getMessageLength()
{
    return messageLength;
}

int Hamming::getEncodedMessageLength()
{
    return encodedMessageLength;
}

int Hamming::getParityBitsLength()
{
    return parityBits;
}

void Hamming::printGMatrix()
{
    print2DArray(g, messageLength, encodedMessageLength);
}

void Hamming::printHMatrix()
{
    print2DArray(h, parityBits, encodedMessageLength);
}