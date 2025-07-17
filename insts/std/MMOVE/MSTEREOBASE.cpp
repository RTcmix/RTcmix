//
// Created by Douglas Scott on 3/30/25.
//

#include "MSTEREOBASE.h"
#include <ugens.h>
#include <rt.h>
#include <rtdefs.h>
#include <string.h>
#include "../MOVE/common.h"

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

extern int g_Nterms[13];				 /* defined in common.cpp */

void MSTEREOBASE::BinauralVector::allocate(int length)
{
    Firtaps = new double[length+1];
    memset(Firtaps, 0, (length+1) * sizeof(double));
    Fircoeffs = new double[length];
}

void MSTEREOBASE::BinauralVector::runFilters(int currentSamp, int len, int pathIndex)
{
    Vector::runFilters(currentSamp, len, pathIndex);
    fir(Sig, currentSamp, g_Nterms[pathIndex], Fircoeffs, Firtaps, len);
}

MSTEREOBASE::MSTEREOBASE() : MBASE(2, 13) {
}

MSTEREOBASE::~MSTEREOBASE() {

}

int MSTEREOBASE::checkOutputChannelCount()
{
    if (outputChannels() != 4)
        return die(name(), "Output must be 4-channel (2 signal, 2 reverb feed).");
    return 0;
}

int MSTEREOBASE::configure() {
    int status = MBASE::configure();
    if (status == 0) {
        for (int ch = 0; ch < m_chans; ++ch) {
            for (int v = 0; v < m_paths; ++v) {
                m_vectors[ch][v]->configure(getBufferSize());
            }
        }
        if (binaural()) {
            status = alloc_firfilters();	// allocates memory for FIRs
        }
    }
    return status;
}

int MSTEREOBASE::alloc_vectors()
{
    // N.B. Create vectors here where we (1) know whether binaural is active and (2) before we call the base class configure
    for (int count = 0; count < m_chans; ++count) {
        VecEntry entry;
        for (int p = 0; p < m_paths; ++p) {
            std::shared_ptr<Vector> vec = binaural() ? std::make_shared<BinauralVector>() : std::make_shared<Vector>();
            entry.push_back(vec);
        }
        m_vectors.push_back(entry);
    }
    return 0;
}

/* -------------------------------------------------------------------------- */
/* The following functions from the original space.c. */
/* -------------------------------------------------------------------------- */

int MSTEREOBASE::alloc_firfilters()
{
    /* allocate memory for FIR filters and zero delays.  This is only
     * done for stereo output.
     */
    try {
        for (int i = 0; i < m_chans; i++) {
            for (int j = 0; j < m_paths; j++) {
                BinauralVector *bvec = (BinauralVector *) m_vectors[i][j].get();
                bvec->allocate(g_Nterms[j]);
            }
        }
    } catch(...) {
        rterror("MBASE (alloc_firfilters/Firtaps)", "Memory failure during setup");
        return -1;
    }
    return 0;
}

/* ----------------------------------------------------------- earfil_set --- */
/* earfil_set is called by place to load coeffs for the fir filter bank
   used in ear, the binaural image filter.  NOTE: Only for stereo!
*/
void
MSTEREOBASE::earfil_set(int flag)
{
    for (int i = 0; i < m_chans; ++i)
        for (int j = 0; j < m_paths; ++j) {
            BinauralVector *bvec = (BinauralVector *) m_vectors[i][j].get();
            setfir(bvec->Theta,
                   g_Nterms[j],
                   flag,
                   bvec->Fircoeffs,
                   bvec->Firtaps);
        }
}

/* ------------------------------------------------------------------ run --- */
int MSTEREOBASE::run()
{
    const int totalSamps = insamps + tapcount;
    int thisFrame = currentFrame();
    const int outChans = outputChannels();
    const bool isBinaural = binaural();

    DBG1(printf("%s::run(): totalSamps = %d\n", name(), totalSamps));

    // this will return chunksamps worth of input, even if we have
    // passed the end of the input (will produce zeros)

    getInput(thisFrame, framesToRun());

    DBG1(printf("getInput(%d, %d) called\n", thisFrame, framesToRun()));

    int bufsamps = getBufferSize();
    const int outputOffset = this->output_offset;

    // loop for required number of output samples
    const int frameCount = framesToRun();

    memset(this->outbuf, 0, frameCount * outChans * sizeof(BUFTYPE));

    int frame = 0;
    while (frame < frameCount) {
        // limit buffer size to end of current pull (chunksamps)
        if (frameCount - frame < bufsamps)
            bufsamps = max(0, frameCount - frame);

        thisFrame = currentFrame();	// store this locally for efficiency

        DBG1(printf("top of main loop: frame = %d  thisFrame = %d  bufsamps = %d\n",
                    frame, thisFrame, bufsamps));
        DBG(printf("input signal:\n"));
        DBG(PrintInput(&in[frame], bufsamps));

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
#if 0
                    DBG(printf("signal [%d][%d] before filters:\n", ch, path));
					DBG(PrintSig(vec->Sig, bufsamps));
#endif
                    vec->runFilters(thisFrame, bufsamps, path);

                    DBG(printf("signal [%d][%d] before rvb:\n", ch, path));
                    DBG(PrintSig(vec->Sig, bufsamps));
//                    DBG(PrintSig(vec->Sig, bufsamps, SIG_THRESH));
                }
            }
            DBG(printf("summing vectors\n"));
            for (int ch = 0; ch < m_chans; ++ch) {
                for (int path = 1; path < m_paths; path++) {
                    SVector vec = m_vectors[ch][path];
                    // sum unscaled reflected paths as global input for RVB.
                    addScaleBuf(&this->m_mixbufs[ch+m_chans][0], vec->Sig, bufsamps, 1.0);
                    if (!isBinaural) {
                        // do cardioid mike effect
                        // add scaled reflected paths to output as early response
                        addScaleBuf(&this->m_mixbufs[ch][0], vec->Sig, bufsamps, vec->MikeAmp);
                    }
                }
            }
#if 0
            DBG(printf("early response L and R:\n"));
            DBG(PrintOutput(&this->m_mixbufs[0][0], bufsamps, outChans, SIG_THRESH));
            DBG(PrintOutput(&this->m_mixbufs[1][0], bufsamps, outChans, SIG_THRESH));
#endif
            if (isBinaural) {
                // copy scaled, filtered reflected paths (reverb input) as the early reponse
                // to the output
                for (int ch = 0; ch < m_chans; ++ch) {
                    copyBuf(&this->m_mixbufs[ch][0], &this->m_mixbufs[ch+m_chans][0], bufsamps);
                }
            }
            /* add the direct signal into the mix bus  */
            for (int ch = 0; ch < m_chans; ++ch) {
                addScaleBuf(&this->m_mixbufs[ch][0], m_vectors[ch][0]->Sig, bufsamps, 1.0);
            }

            /* Now mix this into the output buffer */
            float *outptr = &outbuf[frame * outChans];
            for (int ch = 0; ch < outChans; ++ch) {
                copyBufToOut(&outptr[ch], &this->m_mixbufs[ch][0], 4, bufsamps);
                memset(&this->m_mixbufs[ch][0], 0, bufsamps * sizeof(double));
            }
            DBG(printf("FINAL MIX LEFT CHAN:\n"));
            DBG(PrintOutput(&this->outbuf[frame*outChans], bufsamps, outChans));
        }
        increment(bufsamps);
        frame += bufsamps;
        bufsamps = getBufferSize();		// update
        DBG1(printf("\tmain loop done.  thisFrame now %d\n", currentFrame()));
    }
    DBG1(printf("%s::run done\n\n", name()));
    return frame;
}
