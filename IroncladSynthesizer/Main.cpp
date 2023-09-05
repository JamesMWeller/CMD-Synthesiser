#include<iostream>
using namespace std;

#include "olcNoiseMaker.h"

atomic<double> dFrequencyOutput = 0.0;

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

double MakeNoise(double dTime)
{
	double dOutput = osc(dFrequencyOutput, dTime, selector);

	return dOutput * 0.5; // Volume

}

int main() 
{
	wcout << "Synthesizer" << endl;

	// Get sound hardware
	vector<wstring> devices = olcNoiseMaker<short>::Enumerate();

	// Display findings
	for (auto d : devices) wcout << "Found output device:" << d << endl;
	 
	// Create sound machine
	//using a short can be replaced with:
	//char = 8bit || short = 16bit || int = 32bit
	//Output Device || SampleRate || Channels || Latence Management (Blocks || BlockSample)
	olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);

	//Link noise function with sound machine
	sound.SetUserFunction(MakeNoise);

	double dOctaveBaseFrequency = 110.0; //A2
	double d12thRootOf2 = pow(2.0, 1.0 / 12.0);

	while (1)
	{
		// Keyboard Control

		bool bKeyPressed = false;
		for (int k = 0; k < 15; k++)
		{
			// \xbcL = , || \xbe = .
			if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe"[k])) & 0x8000)
			{
				dFrequencyOutput = dOctaveBaseFrequency * pow(d12thRootOf2, k);
				bKeyPressed = true;
			}
		}

		if (!bKeyPressed)
		{
			dFrequencyOutput = 0.0;
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