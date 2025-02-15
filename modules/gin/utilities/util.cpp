/*==============================================================================
 
 Copyright 2018 by Roland Rabien
 For more information visit www.rabiensoftware.com
 
 ==============================================================================*/

Colour goldenRatioColor (int idx)
{
    double h = std::fmod (idx * 0.618033988749895, 1);
    return Colour (float (h), 0.8f, 0.95f, 1.0f);
}

//==============================================================================
int versionStringToInt (const String& versionString)
{
    StringArray parts;
    parts.addTokens (versionString, ".", "");
    parts.trim();
    parts.removeEmptyStrings();
    
    int res = 0;
    
    for (auto part : parts)
        res = (res << 8) + part.getIntValue();
    
    return res;
}
