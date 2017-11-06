// L6470を用いて回転させた円板の向きを取得する

// get direction of the rotating disc by encoder  
# define PIN_ENC 7
# define PIN_BUSY 9
# define PIN_LED 13

const int numSlit = 8;

boolean encVal_cr;
boolean encVal_prv;

int slitCount;
unsigned long falseSrt;
unsigned long falseEnd;
unsigned long falseTime;
unsigned long trueSrt;
unsigned long trueEnd;
unsigned long trueTime;

boolean initBool;
float slitThd;

void setup() {
        Serial.begin(2000000);
        pinMode(PIN_ENC, INPUT);
        pinMode(PIN_LED, OUTPUT);

        encVal_prv = digitalRead(PIN_ENC);

        initBool = true;
        slitCount = 0;
        falseSrt = micros();
        trueSrt = micros();
        digitalWrite(PIN_LED, LOW);
}

void loop() {
        // get a BUSY flag from a motor driver.
        // now, we use a stepping motor driven by L6470 and be waiting end of the acceleration.
        while( !digitalRead(PIN_BUSY)){ 
               initBool = true;       
               delay(10);
               Serial.println("busy_now");
        }

        // get threshold of the magnification between wide slit and thin slit.
        if( initBool == true ){
                const int initRot = 5;
                unsigned int elemCount;
                const int numElem =  ( numSlit + 1 ) * initRot;
                unsigned long falseNrrw[ numElem ];
                unsigned long falseWid[ numElem ];
                unsigned long trueArray[ numElem ];
                float falseNrrwAve;
                float falseWidAve;

                slitCount = 0;
                elemCount = 0;
                for( int i = 0; i < numElem; i++ ){
                        falseNrrw[ i ] = 0;
                        falseWid[ i ] = 0;
                }

                while( initBool ){
                        encVal_cr = digitalRead(PIN_ENC); // in the slit area = false,  except area = true;
                        if (encVal_cr != encVal_prv){
                                if( encVal_cr ==  false){ // start slit area 
                                        falseSrt = micros();
                                        trueEnd = micros(); 
                                        trueTime = trueEnd - trueSrt;
                                        trueArray[ elemCount ] = trueTime;
                                        }
                                if( encVal_cr == true ){ // end slit area
                                        trueSrt = micros();
                                        falseEnd = micros();
                                        falseTime = falseEnd - falseSrt;
                                        Serial.print( "false: " ); Serial.println( falseTime ); 
                                        Serial.print( "true: " ); Serial.println( trueTime );                                  
                                        slitCount += 1;
                                        if( slitCount <= numSlit+1 ){
                                                // excpet first rotation 
                                        }else if( slitCount == 1 + (numSlit+1) ){
                                                elemCount  = 0;
                                                falseNrrw[0] = falseTime;
                                                falseWid[0] = falseTime;
                                        }else if( slitCount <= numElem + (numSlit+1) ){
                                                elemCount  += 1;
                                                if ( falseTime > falseWid[ elemCount -1 ] * 0.8){
                                                        falseWid[ elemCount ] = falseTime;
                                                        falseNrrw[ elemCount ] = falseNrrw[ elemCount - 1 ];                                                
                                                }
                                                if ( falseTime < falseWid[ elemCount -1 ] * 0.8){
                                                        falseNrrw[ elemCount ] = falseTime;
                                                        falseWid[ elemCount ] = falseWid[ elemCount - 1 ];        
                                                }
                                        }else if( slitCount > numElem + (numSlit+1) ){
                                                initBool = false;                
                                                slitCount = 0;
                                                elemCount = 0;
                                                Serial.println("");
                                                Serial.println( "falseNrrw: "  );
                                                for( int i = 0; i < numElem; i++ ){
                                                        delay(10);
                                                        Serial.println( falseNrrw[ i ] );
                                                }
                                                Serial.println( "falseWid: " ); 
                                                for( int i = 0; i < numElem; i++ ){
                                                        delay(10);
                                                        Serial.println( falseWid[ i ] );
                                                }
                                                Serial.println( "tureTime: " ); 
                                                for( int i = 0; i < numElem; i++ ){
                                                        Serial.println( trueArray[ i ] );
                                                }
                                                float falseNrrwStd = stdev( falseNrrw, numElem ); // standard deviation
                                                float falseWidStd = stdev( falseWid, numElem ); // standard deviation
                                                falseNrrwAve = exoutMean( falseNrrw, falseNrrwStd, numElem ); // mean except outliers
                                                falseWidAve = exoutMean( falseWid, falseWidStd, numElem ); // mean except outliers
                                                slitThd = sqrt( falseWidAve / falseNrrwAve );
                                                Serial.println("falseNrrwStd: "); Serial.println( falseNrrwStd );
                                                Serial.println("falseNrrwAve: "); Serial.println( falseNrrwAve );
                                                Serial.println("falseWidStd: "); Serial.println( falseWidStd );                                       
                                                Serial.println("falseWidAve: "); Serial.println( falseWidAve );
                                                Serial.println("threshold of magnification between falseWid and falseNrrw: "); Serial.println( slitThd );
                                                Serial.println("");
                                         }
                                }
                        }        
                        encVal_prv = encVal_cr;
                }
                slitThd = falseWidAve / slitThd;                
        }

        // get encoder state and count slit
        if( initBool == false ){
                encVal_cr = digitalRead(PIN_ENC); // in the slit area = false,  except area = true;
                if( encVal_cr != encVal_prv ){
                        if ( encVal_cr == false ){
                                falseSrt = micros();
                                trueEnd = micros(); 
                                trueTime = trueEnd - trueSrt;                 
                        }
                        if( encVal_cr == true ){
                                trueSrt = micros();
                                falseEnd = micros();
                                falseTime = falseEnd - falseSrt;
                                if( falseTime  > slitThd ){
                                        slitCount = 0;        
                                }else{
                                        slitCount += 1;
                                        digitalWrite(PIN_LED, HIGH);
                                        delay(1);
                                        digitalWrite(PIN_LED, LOW);                                        
                                }                        
                                Serial.print( "slit: " ); Serial.print( slitCount ); Serial.print( ", " );
                                Serial.print( "false: " ); Serial.println( falseTime );
                        }
                }
                encVal_prv = encVal_cr;
        }
}

float mean( unsigned long x[], int xlength ){
        unsigned long sum = 0;
        float ave = 0;
        for(int i = 0; i < xlength ; i++ ){
                sum += x[ i ];
        }
        return ave = (float)sum / xlength;
}

float stdev( unsigned long x[], int xlength ){
        float ave = mean( x, xlength );
        float var = 0;
        float std = 0;
        for( int i = 0; i < xlength; i++ ){
                var += ( x[ i ] - ave ) * ( x[ i ] - ave );
        }
        var = var / xlength;
        return std = sqrt(var);
}

float exoutMean ( unsigned long x[], float xstdev, int xlength ){
        // except outliers mean
        unsigned long sum = 0;
        float ave = mean( x, xlength );
        int n = 0;
        for( int i = 0; i < xlength; i++ ){
                if( abs( (float)x[ i ] - ave ) <= 2*xstdev ){
                        sum += x[ i ]; 
                        n += 1;                                                                 
                }
        }
        return ave = (float)sum / n ;
}
