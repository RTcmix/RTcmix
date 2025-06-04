//
// Created by Douglas Scott on 3/30/25.
//

#include "QMOVE.h"
#include <ugens.h>
#include <rt.h>
#include "../MOVE/common.h"

//#define SIG_DEBUG 1
//#define debug 1

#ifdef SIG_DEBUG
#define DBG(stmt) { stmt; }
#else
#define DBG(stmt)
#endif

#if defined (MYDEBUG) || defined(SIG_DEBUG) || defined(LOOP_DEBUG)
#define DBG1(stmt) { stmt; }
#else
#define DBG1(stmt)
#endif

extern "C" int get_params(int *cartesian, double *mdiff);

/* ------------------------------------------------------------ makeMOVE --- */
Instrument *makeQMOVE()
{
    QMOVE *inst;

    inst = new QMOVE();
    inst->set_bus_config("QMOVE");

    return inst;
}

#ifndef EMBEDDED
extern Instrument *makeQRVB();	// From QRVB.C

/* ------------------------------------------------------------ rtprofile --- */
void rtprofile()
{
    RT_INTRO("QMOVE", makeQMOVE);
    RT_INTRO("QRVB", makeQRVB);
}
#endif

QMOVE::QMOVE() : MBASE(4, 13), MOVEBASE(RTBUFSAMPS)
{
}

QMOVE::~QMOVE()
{
}

int QMOVE::checkOutputChannelCount()
{
    if (outputChannels() != 8)
        return die(name(), "Output must be 8-channel (4 signal, 4 reverb feed).");
    return 0;
}

int QMOVE::alloc_vectors() {
    for (int ch = 0; ch < m_chans; ++ch) {
        VecEntry entry;
        for (int p = 0; p < m_paths; ++p) {
            entry.push_back(std::make_shared<Vector>());
        }
        m_vectors.push_back(entry);
    }
    return 0;
}

int QMOVE::localInit(double *p, int n_args)
{
    if (n_args < 7)
        return die(name(), "Wrong number of args.");
    m_dist = p[6];
    if (m_dist < 0.0) {
        rtcmix_advise(name(), "Using cartesian coordinate system");
        m_dist *= -1.0;
        m_cartflag = 1;
    }
    m_inchan = n_args > 7 ? (int)p[7] : AVERAGE_CHANS;
    if (alloc_vectors() == DONT_SCHEDULE) {
        return DONT_SCHEDULE;
    }

    // copy global params into instrument
    int ignoreme;
    if (get_params(&ignoreme, &mindiff) < 0)
        return die(name(), "get_params failed.");

    // treat mindiff as update rate in seconds
    if (mindiff > 0.0) {
        m_updateSamps = (int) (SR * mindiff + 0.5);
        if (m_updateSamps <= RTBUFSAMPS)
        {
            setBufferSize(m_updateSamps);
#ifdef debug
            printf("buffer size reset to %d samples\n", getBufferSize());
#endif
        }
        // if update rate is larger than RTBUFSAMPS samples, set buffer size
        // to be some integer fraction of the desired update count, then
        // reset update count to be multiple of this.
        else {
            int divisor = 2;
            int newBufferLen;
            while ((newBufferLen = m_updateSamps / divisor) > RTBUFSAMPS)
                divisor++;
            setBufferSize(newBufferLen);
            m_updateSamps = newBufferLen * divisor;
#ifdef debug
            printf("buffer size reset to %d samples\n", getBufferSize());
#endif
        }
#ifdef debug
        printf("updating every %d samps\n", m_updateSamps);
#endif
    }

    return 0;
}

int QMOVE::finishInit(double *ringdur)
{
#ifdef debug
    printf("finishInit()\n");
#endif
    *ringdur = (float)m_tapsize / SR;	// max possible room delay
    tapcount = updatePosition(0);
    R_old = T_old = -99999999.0;	// force updatePosition() to do its work next time
    return 0;
}


int QMOVE::configure()
{
    int status = MBASE::configure();
    if (status == 0) {
        for (int ch = 0; ch < m_chans; ++ch) {
            for (int v = 0; v < m_paths; ++v) {
                m_vectors[ch][v]->configure(getBufferSize());
            }
        }
    }
    return status;
}

void QMOVE::get_tap(int currentSamp, int chan, int path, int len) {
    SVector vec = m_vectors[chan][path];
    getVecTap(m_tapDelay, m_tapsize, vec->Sig, vec->outloc, &oldOutlocs[chan][path], currentSamp, len);
}

// This gets called every internal buffer's worth of samples.  The actual
// update of source angles and delays only happens if we are due for an update
// (based on the update count) and there has been a change of position

int QMOVE::updatePosition(int currentSamp)
{
    const int totalSamps = insamps + tapcount;
    double p[6];
    double R = update(4, totalSamps);
    double T = update(5, totalSamps);
    int maxtap = tapcount;
    if (R != R_old || T != T_old) {
#ifdef debug
        printf("updatePosition[%d]:\t\tR: %f  T: %f\n", currentSamp, R, T);
#endif
        double yOffets[2];
        yOffets[0] = -m_dist / SQRT_TWO;
        yOffets[1] = -yOffets[0];
        if (roomtrig(R , T, 2 * m_dist / SQRT_TWO, yOffets, m_cartflag)) {
            return (-1);
        }
        // set taps, return max samp
        maxtap = tap_set(binaural());
        int resetFlag = (currentSamp == 0);
        airfil_set(resetFlag);
        mike_set();
        R_old = R;
        T_old = T;
    }
    return maxtap;	// return new maximum delay in samples
}

/* ------------------------------------------------------------------ run --- */
int QMOVE::run()
{
    const int totalSamps = insamps + tapcount;
    int thisFrame = currentFrame();
    const int outChans = outputChannels();

    DBG1(printf("%s::run(): totalSamps = %d\n", name(), totalSamps));

    // this will return chunksamps worth of input, even if we have
    // passed the end of the input (will produce zeros)

    getInput(thisFrame, framesToRun());

    DBG1(printf("getInput(%d, %d) called\n", thisFrame, framesToRun()));

    int bufsamps = getBufferSize();

    // loop for required number of output samples
    const int frameCount = framesToRun();

    clearOutput(frameCount);

    int frame = 0;
    while (frame < frameCount) {
        // limit buffer size to end of current pull (chunksamps)
        if (frameCount - frame < bufsamps)
            bufsamps = max(0, frameCount - frame);

        thisFrame = currentFrame();	// store this locally for efficiency

        DBG1(printf("top of main loop: frame = %d  thisFrame = %d  bufsamps = %d\n",
                    frame, thisFrame, bufsamps));
    //    DBG(printf("input signal for frame %d:\n", frame));
    //    DBG(PrintInput(&in[frame], bufsamps));

        // add signal to delay
        put_tap(thisFrame, &in[frame], bufsamps);

        // if processing input signal or flushing delay lines ...

        if (thisFrame < totalSamps) {
            // limit buffer size of end of input data
            if (totalSamps - thisFrame < bufsamps)
                bufsamps = max(0, totalSamps - thisFrame);

            if ((tapcount = updatePosition(thisFrame)) < 0)
                return -1;

            DBG1(printf("  vector loop: bufsamps = %d\n", bufsamps));
            for (int ch = 0; ch < m_chans; ch++) {
                for (int path = 0; path < m_paths; path++) {
                    SVector vec = m_vectors[ch][path];
                    /* get delayed samps */
                    get_tap(thisFrame, ch, path, bufsamps);
#if 1
                    DBG(printf("signal [ch %d][path %d] before filters:\n", ch, path));
					DBG(PrintSig(vec->Sig, bufsamps));
#endif
                    vec->runFilters(thisFrame, bufsamps, path);

                    DBG(printf("signal [ch %d][path %d] before rvb:\n", ch, path));
                    DBG(PrintSig(vec->Sig, bufsamps));
                }
            }
            DBG(printf("summing %d sets of early response vectors\n", m_chans));
            for (int ch = 0; ch < m_chans; ++ch) {
                for (int path = 1; path < m_paths; path++) {
                    SVector vec = m_vectors[ch][path];
                    // sum unscaled reflected paths as global input for RVB via mixbufs[4,5,6,7].
                    addScaleBuf(&this->m_mixbufs[ch+m_chans][0], vec->Sig, bufsamps, 1.0);
                    // sum scaled reflected paths to output as early response via mixbufs[0,1,2,3]
                    addScaleBuf(&this->m_mixbufs[ch][0], vec->Sig, bufsamps, vec->MikeAmp);
                }
            }
#if 0
            DBG(printf("early response LF and RF:\n"));
            DBG(PrintOutput(&this->m_mixbufs[0][0], bufsamps, outChans, SIG_THRESH));
            DBG(PrintOutput(&this->m_mixbufs[1][0], bufsamps, outChans, SIG_THRESH));
#endif
            /* add the direct signal in vectors[*][0] into mixbufs[0,1,2,3]  */
            for (int ch = 0; ch < m_chans; ++ch) {
                addScaleBuf(&this->m_mixbufs[ch][0], m_vectors[ch][0]->Sig, bufsamps, 1.0);
            }

            /* Now mix these into the 8-channel interleaved output buffer */
            float *outptr = &outbuf[frame * outChans];
            for (int ch = 0; ch < outChans; ++ch) {
                copyBufToOut(&outptr[ch], &this->m_mixbufs[ch][0], outChans, bufsamps);
                memset(&this->m_mixbufs[ch][0], 0, bufsamps * sizeof(double));
            }
            DBG(printf("FINAL MIX EACH CHAN:\n"));
            DBG(printf("LF\n"));
            DBG(PrintOutput(&outptr[0], bufsamps, outChans));
            DBG(printf("RF\n"));
            DBG(PrintOutput(&outptr[1], bufsamps, outChans));
            DBG(printf("LB\n"));
            DBG(PrintOutput(&outptr[2], bufsamps, outChans));
            DBG(printf("RB\n"));
            DBG(PrintOutput(&outptr[3], bufsamps, outChans));
            DBG(printf("DONE DISPLAYING FINAL MIX\n"));
        }
        increment(bufsamps);
        frame += bufsamps;
        bufsamps = getBufferSize();		// update
        DBG1(printf("\tmain loop done.  currentFrame now %d\n", currentFrame()));
    }
    DBG1(printf("%s::run done\n\n", name()));
    return frame;
}