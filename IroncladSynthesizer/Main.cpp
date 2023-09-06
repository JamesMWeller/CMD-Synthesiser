#include<iostream>
using namespace std;

#include "olcNoiseMaker.h"



double selector = 0;

// Converts frequency (Hz) to angular velocity
double w(double dHertz) { return dHertz * 2 * PI;}

double osc(double dHertz, double dTime, int nType)
{
	switch (nType)
	{

	case 0:	//SinWave
		return sin(w(dHertz) * dTime);

	case 1:	//SquareWave
		return sin(w(dHertz) * dTime)> 0.0 ? 1.0 : -1.0;

	case 2: //TriangleWave
		return asin(sin(w(dHertz) * dTime)) * (2.0/PI);

	case 3: // Saw Wave (Analogue / Warm / Slow)
	{
		double dOutput = 0.0;

		for (double n = 1.0; n < 100.0; n++)
			dOutput += (sin(n * w(dHertz) * dTime)) / n;
		return dOutput * (2.0 / PI);
	}
	case 4: // Saw Wave (Optimised / Harsh / Fast)
	{
		return (2.0 / PI) * (dHertz * PI * fmod(dTime, 1.0 / dHertz) - (PI / 2.0));
	}
	case 5: // Pseudo Random Noise - WARNING loud.
		return 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0;
	default:
		return 0.0;
	}
}

struct sEnvelopeADSR
{

	// Struct Variables
	double dAttackTime;
	double dDecayTime;
	double dReleaseTime;

	double dSustainAmplitude;
	double dStartAmplitude;

	double dTriggerOnTime;
	double dTriggerOffTime;

	bool bNoteOn;

	//Instialising Varaibles
	sEnvelopeADSR()
	{
		dAttackTime = 0.10;
		dDecayTime = 0.01;
		dStartAmplitude = 1.0;
		dSustainAmplitude = 0.8;
		dReleaseTime = 0.20;
		bNoteOn = false;
		dTriggerOffTime = 0.0;
		dTriggerOnTime = 0.0;
	}

	void NoteOn(double dTimeOn)
	{
		dTriggerOnTime = dTimeOn;
		bNoteOn = true;
	}

	void NoteOff(double dTimeOff)
	{
		dTriggerOffTime = dTimeOff;
		bNoteOn = false;
	}

	double GetAmplitude(double dTime)
	{
		double dAmplitude = 0.0;
		double dLifeTime = dTime - dTriggerOnTime;

		if (bNoteOn)
		{
			if (dLifeTime <= dAttackTime)
			{
				//Attack
				dAmplitude = (dLifeTime / dAttackTime) * dStartAmplitude;
			}

			if (dLifeTime > dAttackTime && dLifeTime <= (dAttackTime + dDecayTime))
			{
				//Decay
				dAmplitude = ((dLifeTime - dAttackTime) / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude;
			}

			if (dLifeTime > (dAttackTime + dDecayTime))
			{
				// Sustain
				dAmplitude = dSustainAmplitude;
			}
		}
		else
		{
			//Release
			dAmplitude = ((dTime - dTriggerOffTime) / dReleaseTime) * (0.0 - dSustainAmplitude) + dSustainAmplitude;
		}

		
		if (dAmplitude <= 0.0001)
			dAmplitude = 0.0;

		return dAmplitude;
	}


};

//Global Variables
atomic<double> dFrequencyOutput = 0.0;
double dOctaveBaseFrequency = 110.0;
double d12thRootOf2 = pow(2.0, 1.0 / 12.0);
sEnvelopeADSR envelope;

double MakeNoise(double dTime)
{
	//double dOutput = envelope.GetAmplitude(dTime) * osc(dFrequencyOutput, dTime, selector);

	double dOutput = envelope.GetAmplitude(dTime) *
		(
			+ osc(dFrequencyOutput * 0.5, dTime, 3)
			+ osc(dFrequencyOutput * 1.0, dTime,1)
		);

	return dOutput * 0.5; // Volume

}

int main() 
{
	wcout << "Synthesizer" << endl;

	// Get sound hardware
	vector<wstring> devices = olcNoiseMaker<short>::Enumerate();

	// Display findings
	for (auto d : devices) wcout << "Found output device:" << d << endl;

	// Display a keyboard
	wcout << endl <<
		"|   |   |   |   |   | |   |   |   |   | |   | |   |   |   |" << endl <<
		"|   | S |   |   | F | | G |   |   | J | | K | | L |   |   |" << endl <<
		"|   |___|   |   |___| |___|   |   |___| |___| |___|   |   |__" << endl <<
		"|     |     |     |     |     |     |     |     |     |     |" << endl <<
		"|  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |" << endl <<
		"|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << endl << endl;
	 
	// Create sound machine
	//using a short can be replaced with:
	//char = 8bit || short = 16bit || int = 32bit
	//Output Device || SampleRate || Channels || Latence Management (Blocks || BlockSample)
	olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);

	//Link noise function with sound machine
	sound.SetUserFunction(MakeNoise);

	int nCurrentKey = -1;	
	bool bKeyPressed = false;
	while (1)
	{
		bKeyPressed = false;
		for (int k = 0; k < 16; k++)
		{
			if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe\xbf"[k])) & 0x8000)
			{
				if (nCurrentKey != k)
				{					
					dFrequencyOutput = dOctaveBaseFrequency * pow(d12thRootOf2, k);
					envelope.NoteOn(sound.GetTime());					
					nCurrentKey = k;
				}

				bKeyPressed = true;
			}
		}

		if (!bKeyPressed)
		{	
			if (nCurrentKey != -1)
			{
				envelope.NoteOff(sound.GetTime());
				nCurrentKey = -1;
			}
		}

		// Oscillator Selector
		for (int s = 0; s < 9; s++)
		{
			if (GetAsyncKeyState((unsigned char)("123456789"[s])) & 0x8000)
			{
				selector = s;
			}
		}
	}

	return 0;
} 