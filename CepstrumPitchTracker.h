/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */
/*
    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _CEPSTRUM_PITCH_H_
#define _CEPSTRUM_PITCH_H_

#include <vamp-sdk/Plugin.h>

class CepstrumPitchTracker : public Vamp::Plugin
{
public:
    CepstrumPitchTracker(float inputSampleRate);
    virtual ~CepstrumPitchTracker();

    std::string getIdentifier() const;
    std::string getName() const;
    std::string getDescription() const;
    std::string getMaker() const;
    int getPluginVersion() const;
    std::string getCopyright() const;

    InputDomain getInputDomain() const;
    size_t getPreferredBlockSize() const;
    size_t getPreferredStepSize() const;
    size_t getMinChannelCount() const;
    size_t getMaxChannelCount() const;

    ParameterList getParameterDescriptors() const;
    float getParameter(std::string identifier) const;
    void setParameter(std::string identifier, float value);

    ProgramList getPrograms() const;
    std::string getCurrentProgram() const;
    void selectProgram(std::string name);

    OutputList getOutputDescriptors() const;

    bool initialise(size_t channels, size_t stepSize, size_t blockSize);
    void reset();

    FeatureSet process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp);

    FeatureSet getRemainingFeatures();

protected:
    size_t m_channels;
    size_t m_stepSize;
    size_t m_blockSize;
    float m_fmin;
    float m_fmax;
    int m_vflen;

    int m_binFrom;
    int m_binTo;
    int m_bins; // count of "interesting" bins, those returned in m_cepOutput

    class Hypothesis {

    public:
        struct Estimate {
            double freq;
            Vamp::RealTime time;
            double confidence;
        };
        typedef std::vector<Estimate> Estimates;

        struct Note {
            double freq;
            Vamp::RealTime time;
            Vamp::RealTime duration;
        };
        
        Hypothesis();
        ~Hypothesis();

        enum State {
            New,
            Provisional,
            Satisfied,
            Rejected,
            Expired
        };

        bool test(Estimate);

        void advanceTime();

        State getState();

        int getPendingLength();
        Estimates getAcceptedEstimates();
        Note getAveragedNote();

        void addFeatures(FeatureSet &fs);

    private:
        bool isWithinTolerance(Estimate);
        bool isSatisfied();
        double getMeanFrequency();

        State m_state;
        Estimates m_pending;
        int m_age;
    };

    typedef std::vector<Hypothesis> Hypotheses;
    Hypotheses m_possible;
    Hypothesis m_accepted;

    void filter(const double *in, double *out);
    void fft(unsigned int n, bool inverse,
             double *ri, double *ii, double *ro, double *io);
};

#endif
