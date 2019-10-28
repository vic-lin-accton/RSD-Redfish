#ifndef ACC_SYS_HELPER_HPP
#define ACC_SYS_HELPER_HPP
 
#include <iostream>
#include <fstream>
#include <istream>
#include <string>
#include <ostream>
#include <string>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <linux/unistd.h>
#include <linux/kernel.h>

#include <thread>
extern char *__progname;
namespace acc_sys_helper {
 
    using namespace std;

    constexpr const size_t MAC_ADDRESS_LENGTH = 18;
    constexpr const size_t  BUFFER_SIZE = 512;
	
    class HelperTools
    {
        public:
            void exec_shell(const char* cmd, char * result_a);

    };

    class CPUData
    {
        public:
            void ReadData(const std::string & line);
            
            std::size_t GetActiveTime() const;
            std::size_t GetIdleTime() const;
            std::size_t GetStateTime(unsigned int state) const;
            std::size_t GetTotalTime() const;
            
            const std::string & GetLabel() const;
       
            static bool IsDataCPUStats(const std::string & line);
        
        public:
            enum CPUStates
            {
                S_USER = 0,
                S_NICE,
                S_SYSTEM,
                S_IDLE,
                S_IOWAIT,
                S_IRQ,
                S_SOFTIRQ,
                S_STEAL,
                S_GUEST,
                S_GUEST_NICE,
                
                NUM_CPU_STATES
            };
        
        private:
			
            static const std::string STR_CPU;
            static const std::string STR_TOT;
            
            static const std::size_t LEN_STR_CPU;
            
            std::string mLabel = {};
            
            std::size_t mTimes[NUM_CPU_STATES];
    };
    
    inline 	std::size_t CPUData::GetActiveTime() const
    {
    	return	mTimes[S_USER] +
    			mTimes[S_NICE] +
    			mTimes[S_SYSTEM] +
    			mTimes[S_IRQ] +
    			mTimes[S_SOFTIRQ] +
    			mTimes[S_STEAL] +
    			mTimes[S_GUEST] +
    			mTimes[S_GUEST_NICE];
    }
    
    inline std::size_t CPUData::GetIdleTime() const
    {
    	return mTimes[S_IDLE] + mTimes[S_IOWAIT];
    }
    
    inline std::size_t CPUData::GetStateTime(unsigned int state) const
    {
    	if(state < NUM_CPU_STATES)
    		return mTimes[state];
    	else
    		return 0;
    }
    
    inline std::size_t CPUData::GetTotalTime() const
    {
    	return	mTimes[S_USER] +
    			mTimes[S_NICE] +
    			mTimes[S_SYSTEM] +
    			mTimes[S_IDLE] +
    			mTimes[S_IOWAIT] +
    			mTimes[S_IRQ] +
    			mTimes[S_SOFTIRQ] +
    			mTimes[S_STEAL] +
    			mTimes[S_GUEST] +
    			mTimes[S_GUEST_NICE];
    }
    
    inline const std::string & CPUData::GetLabel() const { return mLabel; }
    
    inline bool CPUData::IsDataCPUStats(const std::string & line)
    {
    	return (!line.compare(0, LEN_STR_CPU, STR_CPU));
    }
	
    class CPU
    {
        public:
			
            CPU();
		
            std::size_t GetNumEntries() const;
            
            std::size_t GetActiveTimeTotal() const;
            std::size_t GetActiveTime(unsigned int cpu) const;
            
            std::size_t GetIdleTimeTotal() const;
            std::size_t GetIdleTime(unsigned int cpu) const;
            
            std::size_t GetStateTimeTotal(unsigned int state) const;
            std::size_t GetStateTime(unsigned int state, unsigned int cpu) const;
            
            std::size_t GetTotalTimeTotal() const;
            std::size_t GetTotalTime(unsigned int cpu) const;
    
            const char * GetLabelTotal() const;
            const char * GetLabel(unsigned int cpu) const;
			
        private:

            static const int INDEX_TOT;
            std::vector<CPUData> mEntries ={};			

    };

    inline std::size_t CPU::GetNumEntries() const { return mEntries.size() - 1; }
    
    inline const char * CPU::GetLabelTotal() const { return mEntries[INDEX_TOT].GetLabel().c_str(); }
    
    inline const char * CPU::GetLabel(unsigned int cpu) const
    {
    	++cpu;
    
    	if(cpu < mEntries.size())
    		return mEntries[cpu].GetLabel().c_str();
    	else
    		return nullptr;
    }
    
    inline std::size_t CPU::GetActiveTimeTotal() const { return mEntries[INDEX_TOT].GetActiveTime(); }
    
    inline std::size_t CPU::GetActiveTime(unsigned int cpu) const
    {
    	++cpu;
    
    	if(cpu < mEntries.size())
    		return mEntries[cpu].GetActiveTime();
    	else
    		return 0;
    }
    
    inline std::size_t CPU::GetIdleTimeTotal() const { return mEntries[INDEX_TOT].GetIdleTime(); }
    
    inline std::size_t CPU::GetIdleTime(unsigned int cpu) const
    {
    	++cpu;
    
    	if(cpu < mEntries.size())
    		return mEntries[cpu].GetIdleTime();
    	else
    		return 0;
    }
    
    inline std::size_t CPU::GetStateTimeTotal(unsigned int state) const { return mEntries[INDEX_TOT].GetStateTime(state); }
    
    inline std::size_t CPU::GetStateTime(unsigned int state, unsigned int cpu) const
    {
    	++cpu;
    
    	if(cpu < mEntries.size())
    		return mEntries[cpu].GetStateTime(state);
    	else
    		return 0;
    }
    
    inline std::size_t CPU::GetTotalTimeTotal() const { return mEntries[INDEX_TOT].GetTotalTime(); }
    
    inline std::size_t CPU::GetTotalTime(unsigned int cpu) const
    {
    	++cpu;
    
    	if(cpu < mEntries.size())
    		return mEntries[cpu].GetTotalTime();
    	else
    		return 0;
    }

    class CPUStatsPrinter
    {
        public:
            CPUStatsPrinter(const CPU & s1, const CPU & s2);
			
            void PrintActivePercentageTotal();
            void PrintActivePercentageCPU(unsigned int cpu);
            void PrintActivePercentageAll();
            
            void PrintStatePercentageTotal(unsigned int state);
            void PrintStatePercentageCPU(unsigned int state, unsigned int cpu);
            void PrintStatePercentageAll(unsigned int state);
            
            void PrintFullStatePercentageTotal();
            void PrintFullStatePercentageCPU(unsigned int cpu);
            void PrintFullStatePercentageAll();

            void PrintStatePercentageNoLabelTotal(unsigned int state);
            void PrintStatePercentageNoLabelCPU(unsigned int state, unsigned int cpu);
            
            void SetPrecision(unsigned int prec);            
            void SetVerbose(bool val);
        
            float GetPercActiveTotal();
            float GetPercActive(unsigned int cpu);
            float GetPercStateTotal(unsigned int state);
            float GetPercState(unsigned int state, unsigned int cpu);

            float m_ActivePercentageTotal         = 0;
            float m_ActivePercentageIndividual = 0;
            float m_PercStateTotal                      =0;			
		
            static const int CPU_LABEL_W;
            static const int STATE_PERC_BASE_W;
            
            static const char * STR_STATES[CPUData::NUM_CPU_STATES];
            
            const CPU & mS1 ={};
            const CPU & mS2 ={};
        
        unsigned int mPrecision;
        
        bool mVerbose;
    };
    
    inline void CPUStatsPrinter::SetPrecision(unsigned int prec) { mPrecision = prec; }
    inline void CPUStatsPrinter::SetVerbose(bool val) { mVerbose = val; }
}

#endif
