// get direction of the rotating disc by encoder  

# define PIN_ENC_1 6
# define PIN_ENC_2 7
# define PIN_BUSY 9
# define PIN_LED 13

const int numSlit = 8;

boolean encVal1_cr;
boolean encVal1_prv;
boolean encVal2_cr;
boolean encVal2_prv;

int slitCount;
unsigned long falseSrt;
unsigned long falseEnd;
unsigned long falseTime;
// unsigned long trueSrt;
// unsigned long trueEnd;
// unsigned long trueTime;

boolean initBool;
float slitThd;

void setup() {
        Serial.begin(2000000);
        pinMode(PIN_ENC_1, INPUT);
        pinMode(PIN_ENC_2, INPUT);
        pinMode(PIN_LED, OUTPUT);

        encVal1_prv = digitalRead(PIN_ENC_1);
        encVal2_prv = digitalRead(PIN_ENC_2);

        initBool = true;
        slitCount = 0;
        falseSrt = micros();   
       // trueSrt = micros();     
        digitalWrite(PIN_LED, LOW);
}

void loop() {
        // get a BUSY flag from a motor driver.
        while( !digitalRead(PIN_BUSY)){ 
               // now, we use a stepping motor driven by L6470 and be waiting end of the acceleration.
               initBool = true;       
               delay(10);
               Serial.println("busy_now");
        }

        // get threshold of the magnification between wide slit and thin slit.
        if( initBool == true ){
                unsigned int elemCount;
                const int initRot = 5;
                const int numElem =  ( numSlit + 1 ) * initRot;
                unsigned long falseNrrw[ numElem ];
                unsigned long falseWid[ numElem ];
                float falseNrrwAve;
                float falseWidAve;
                //unsigned long trueArray[ numElem ];

                slitCount = 0;
                elemCount = 0;
                for( int i = 0; i < numElem; i++ ){
                        falseNrrw[ i ] = 0;
                        falseWid[ i ] = 0;
                }

                while( initBool ){
                        encVal2_cr = digitalRead(PIN_ENC_2); // in the slit area = false,  except area = true;
                        if (encVal2_cr != encVal2_prv){
                                if( encVal2_cr ==  false){ // start slit area 
                                        falseSrt = micros();
                                        //trueEnd = micros(); 
                                        //trueTime = trueEnd - trueSrt;
                                        //trueArray[ elemCount ] = trueTime;
                                        }
                                if( encVal2_cr == true ){ // end slit area
                                        //trueSrt = micros();
                                        falseEnd = micros();
                                        falseTime = falseEnd - falseSrt;
                                        Serial.print( "false: " ); Serial.println( falseTime ); 
                                        //Serial.print( "true: " ); Serial.println( trueTime );                                  
                                        slitCount += 1;
                                        if( slitCount <= numSlit+1 ){
                                                // excpet first rotation 
                                        }else if( slitCount == 1 + (numSlit+1) ){
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
                                                //Serial.println( "tureTime: " ); 
                                                //for( int i = 0; i < numElem; i++ ){
                                                //        Serial.println( trueArray[ i ] );
                                                //}
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
                        encVal2_prv = encVal2_cr;
                }
                slitThd = falseWidAve / slitThd;                
        }

        // get the current states of photo interrupter.
        encVal1_cr = digitalRead(PIN_ENC_1); // in the slit area = false,  except area = true;
        encVal2_cr = digitalRead(PIN_ENC_2); // in the slit area = false,  except area = true;

        //
        if( initBool == false ){
                if( encVal2_cr != encVal2_prv ){
                        if ( encVal2_cr == false ){
                                falseSrt = micros();
                                // trueEnd = micros(); 
                                // trueTime = trueEnd - trueSrt;                 
                                // Serial.print( "true: " );          
                                // Serial.println( trueTime ); 
                        }
                        if( encVal2_cr == true ){
                                // trueSrt = micros();
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
        }
        encVal2_prv = encVal2_cr;
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
        //float exoutAve = 0;
        int n = 0;
        for( int i = 0; i < xlength; i++ ){
                if( abs( (float)x[ i ] - ave ) <= 2*xstdev ){
                        sum += x[ i ]; 
                        n += 1;                                                                 
                }
        }
        return ave = (float)sum / n ;
}
