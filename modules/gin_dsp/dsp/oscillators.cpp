/*
 ==============================================================================
 
 This file is part of the GIN library.
 Copyright (c) 2019 - Roland Rabien.
 
 ==============================================================================
 */


#include "adsr.h"
#include <cmath>

//==============================================================================
inline bool oddEven (int x)
{
    return (x % 2 == 0) ? 1 : -1;
}

//==============================================================================
double sine (double phase, double, double)
{
    return std::sin (phase * 2 * MathConstants<double>::pi);
}

double triangle (double phase, double freq, double sampleRate)
{
    double sum = 0;
    int k = 1;
    while (freq * k < sampleRate / 2)
    {
        sum += std::pow (-1, (k - 1) / 2.0f) / (k * k) * std::sin (k * (phase * 2 * MathConstants<double>::pi));
        k += 2;
    }
    return float (8.0f / (float_Pi * float_Pi) * sum);
};

double sawUp (double phase, double freq, double sampleRate)
{
    double sum = 0;
    int k = 1;
    while (freq * k < sampleRate / 2)
    {
        sum += oddEven (k) * std::sin (k * (phase * 2 * MathConstants<double>::pi)) / k;
        k++;
    }
    return float (-2.0f / float_Pi * sum);
};

double sawDown (double phase, double freq, double sampleRate)
{
    double sum = 0;
    int k = 1;
    while (freq * k < sampleRate / 2)
    {
        sum += oddEven (k) * std::sin (k * (phase * 2 * MathConstants<double>::pi)) / k;
        k++;
    }
    return float (2.0f / float_Pi * sum);
};

double pulse (double phase, double pw, double freq, double sampleRate)
{
    if (pw == 0.5)
    {
        double sum = 0;
        int i = 1;
        while (freq * (2 * i - 1) < sampleRate / 2)
        {
            sum += std::sin ((2 * i - 1) * (phase * 2 * MathConstants<double>::pi)) / ((2 * i - 1));
            i++;
        }
        
        return float (4.0f / float_Pi * sum);
    }
    else
    {
        pw = jlimit (0.05, 0.95, pw);
        return sawUp (phase + 0.5 * pw, freq, sampleRate) - sawUp (phase - 0.5 * pw, freq, sampleRate);
    }
};

double squareWave (double phase, double freq, double sampleRate)
{
    double sum = 0;
    int i = 1;
    while (freq * (2 * i - 1) < sampleRate / 2)
    {
        sum += std::sin ((2 * i - 1) * (phase * 2 * MathConstants<double>::pi)) / ((2 * i - 1));
        i++;
    }
    
    return float (4.0f / float_Pi * sum);
};

double noise()
{
    const float mean = 0.0f;
    const float stddev = 0.1f;
    
    static std::default_random_engine generator;
    static std::normal_distribution<float> dist (mean, stddev);
    
    return dist (generator);
};

//==============================================================================
BandLimitedLookupTable::BandLimitedLookupTable (std::function<double (double, double, double)> function, double sampleRate, int notesPerTable_, int tableSize)
    : notesPerTable (notesPerTable_)
{
    for (double note = notesPerTable + 0.5; note < 127.0; note += notesPerTable)
    {
        auto freq = getMidiNoteInHertz (note);
        
        auto func = [function, freq, sampleRate] (float phase)
        {
            return function (phase, freq, sampleRate);
        };
        
        tables.add (new juce::dsp::LookupTableTransform<float> (func, 0.0f, 1.0f, (size_t) tableSize));
    }
}

//==============================================================================
BandLimitedLookupTables::BandLimitedLookupTables (double sampleRate)
  : sineTable (sine, sampleRate, 128),
    sawUpTable (sawUp, sampleRate, 6),
    sawDownTable (sawDown, sampleRate, 6),
    triangleTable (triangle, sampleRate, 6)
{
    
}

float BandLimitedLookupTables::processSine (float phase)
{
    return sineTable.tables[0]->processSampleUnchecked (phase);
}

float BandLimitedLookupTables::processTriangle (float note, float phase)
{
    int tableIndex = jlimit (0, triangleTable.tables.size() - 1, int ((note - 0.5) / triangleTable.notesPerTable));
    return triangleTable.tables[tableIndex]->processSampleUnchecked (phase);
}

float BandLimitedLookupTables::processSawUp (float note, float phase)
{
    int tableIndex = jlimit (0, sawUpTable.tables.size() - 1, int ((note - 0.5) / sawUpTable.notesPerTable));
    return sawUpTable.tables[tableIndex]->processSampleUnchecked (phase);
}

float BandLimitedLookupTables::processSawDown (float note, float phase)
{
    int tableIndex = jlimit (0, sawDownTable.tables.size() - 1, int ((note - 0.5) / sawDownTable.notesPerTable));
    return sawDownTable.tables[tableIndex]->processSampleUnchecked (phase);
}

float BandLimitedLookupTables::processSquare (float note, float phase)
{
    float phaseUp   = phase + 0.25f;
    float phaseDown = phase - 0.25f;
    
    if (phaseUp   > 1.0f) phaseUp   -= 1.0f;
    if (phaseDown < 0.0f) phaseDown += 1.0f;
    
    int tableIndex = jlimit (0, sawDownTable.tables.size() - 1, int ((note - 0.5) / sawDownTable.notesPerTable));
    return sawDownTable.tables[tableIndex]->processSampleUnchecked (phaseDown) +
           sawUpTable.tables[tableIndex]->processSampleUnchecked (phaseUp);
}

float BandLimitedLookupTables::processPulse (float note, float phase, float pw)
{
    float phaseUp   = phase + 0.5f * pw;
    float phaseDown = phase - 0.5f * pw;
    
    if (phaseUp   > 1.0f) phaseUp   -= 1.0f;
    if (phaseDown < 0.0f) phaseDown += 1.0f;
    
    int tableIndex = jlimit (0, sawDownTable.tables.size() - 1, int ((note - 0.5) / sawDownTable.notesPerTable));
    return sawDownTable.tables[tableIndex]->processSampleUnchecked (phaseDown) +
           sawUpTable.tables[tableIndex]->processSampleUnchecked (phaseUp);
}
