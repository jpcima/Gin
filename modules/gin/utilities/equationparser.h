/*==============================================================================

Copyright 2018 by Roland Rabien
For more information visit www.rabiensoftware.com

==============================================================================*/

#pragma once

class EquationParser
{
public:
    EquationParser();
    EquationParser (juce::String equation);
    ~EquationParser();
    
    void setEquation (juce::String equation);
    void addVariable (juce::String name, double* value);
    
    void addConstant (juce::String name, double value);
    
    void addFunction (juce::String name, std::function<double (int id)> fun);
    void addFunction (juce::String name, std::function<double (int id, double)> fun);
    void addFunction (juce::String name, std::function<double (int id, double, double)> fun);
    void addFunction (juce::String name, std::function<double (int id, double, double, double)> fun);
    void addFunction (juce::String name, std::function<double (int id, double, double, double, double)> fun);

    double evaluate();
    
private:
    class Callback
    {
    public:
        virtual ~Callback() = default;
    };
    class Callback0;
    class Callback1;
    class Callback2;
    class Callback3;
    class Callback4;
    
    OwnedArray<Callback> callbacks;
    
    class EquationParserImpl;
    std::unique_ptr<EquationParserImpl> impl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EquationParser)
};
