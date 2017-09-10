// get direction of the rotating disc by encoder  

# define PIN_ENC_1 6
# define PIN_ENC_2 7
# define PIN_BUSY 9
# define PIN_LED 13

const int numSlit = 6;

boolean encVal1_cr;
boolean encVal1_prv;
boolean encVal2_cr;
boolean encVal2_prv;

int slitCount;
unsigned long trueSrt;
unsigned long trueEnd;
unsigned long trueTime;
unsigned long falseSrt;
unsigned long falseEnd;
unsigned long falseTime;

boolean initBool;
unsigned int elemCount;
const int numElem =  ( numSlit + 1 ) * 10;
unsigned long falseMin[ numElem ];
unsigned long falseMax[ numElem ];
unsigned long trueArray[ numElem ];
float falseMinAve;
float falseMaxAve;
float slitThd;

void setup() {
        Serial.begin(2000000);
        pinMode(PIN_ENC_1, INPUT);
        pinMode(PIN_ENC_2, INPUT);
        pinMode(PIN_LED, OUTPUT);

        encVal1_prv = digitalRead(PIN_ENC_1);
        encVal2_prv = digitalRead(PIN_ENC_2);

        slitCount = 0;
        elemCount = 0;
        initBool = true;
        digitalWrite(PIN_LED, LOW);
}

void loop() {
        // get a BUSY flag from a motor driver.
        // now, we use a stepping motor driven by L6470 and be waiting end of the acceleration.
        while( !digitalRead(PIN_BUSY)){ 
                slitCount = 0;
                elemCount = 0;
                initBool = true;       
                falseSrt = micros();   
                trueSrt = micros();     
                for( int i = 0; i < numElem; i++ ){
                        falseMin[ i ] = 0;
                        falseMax[ i ] = 0;
                }
        }  

        // get the current states of photo interrupter.
        encVal1_cr = digitalRead(PIN_ENC_1); // in the slit area = false,  except area = true;
        encVal2_cr = digitalRead(PIN_ENC_2); // in the slit area = false,  except area = true;

        // get threshold of the magnification between wide slit and thin slit.
        if( initBool == true ){
                if (encVal2_cr != encVal2_prv){
                        if( encVal2_cr ==  false){ // start slit area 
                                falseSrt = micros();
                                trueEnd = micros(); 
                                trueTime = trueEnd - trueSrt;
                                trueArray[ elemCount ] = trueTime;
                        }
                        if( encVal2_cr == true ){ // end slit area
                                trueSrt = micros();
                                falseEnd = micros();
                                falseTime = falseEnd - falseSrt;
                                Serial.print( "false: " ); Serial.print( falseTime ); Serial.print( ", " );
                                Serial.print( "true: " ); Serial.println( trueTime );                                  
                                slitCount += 1;
                                if( slitCount == 1 ){
                                        elemCount = 0;
                                        falseMin[0] = falseTime;
                                        falseMax[0] = falseTime;
                                }else if( slitCount <= numElem ){
                                        elemCount  += 1;
                                        if ( falseTime > falseMax[ elemCount -1 ] ){
                                                falseMax[ elemCount ] = falseTime;
                                                falseMin[ elemCount ] = falseMin[ elemCount - 1 ];                                                
                                        }
                                        if ( falseTime < falseMax[ elemCount -1 ] ){
                                                falseMin[ elemCount ] = falseTime;
                                                falseMax[ elemCount ] = falseMax[ elemCount - 1 ];        
                                        }
                                }else if( slitCount > numElem ){
                                        slitCount = 0;
                                        elemCount = 0;                                        
                                        initBool = false;
                                        Serial.println("");
                                        Serial.println( "falseMin: "  );
                                        for( int i = 0; i < numElem; i++ ){
                                                Serial.println( falseMin[ i ] );
                                        }
                                        Serial.println( "falseMax: " ); 
                                        for( int i = 0; i < numElem; i++ ){
                                                Serial.println( falseMax[ i ] );
                                        }
                                        Serial.println( "tureTime: " ); 
                                        for( int i = 0; i < numElem; i++ ){
                                                Serial.println( trueArray[ i ] );
                                        }                                        
                                        unsigned long _falseMin[ numElem - ( numSlit + 1 ) ]; // except first rotation
                                        unsigned long _falseMax[ numElem - ( numSlit + 1 ) ]; // except first rotation
                                        for( int i = 0; i < numElem - ( numSlit + 1 ); i++ ){
                                                _falseMin[ i ] = falseMin[ i + ( numSlit + 1 ) ];     
                                                _falseMax[ i ] = falseMax[ i + ( numSlit + 1 ) ];
                                        }
                                        float falseMinStd = stdev( _falseMin, numElem - ( numSlit + 1 ) ); // standard deviation
                                        float falseMaxStd = stdev( _falseMax, numElem - ( numSlit + 1 ) ); // standard deviation
                                        falseMinAve = exOutMean( _falseMin, falseMinStd, numElem - ( numSlit + 1 )); // mean except outliers
                                        falseMaxAve = exOutMean( _falseMax, falseMaxStd, numElem - ( numSlit + 1 )); // mean except outliers
                                        slitThd = sqrt( falseMaxAve / falseMinAve );
                                        Serial.println("falseMinStd: "); Serial.println( falseMinStd );
                                        Serial.println("falseMinAve: "); Serial.println( falseMinAve );
                                        Serial.println("falseMaxStd: "); Serial.println( falseMaxStd );                                       
                                        Serial.println("falseMaxAve: "); Serial.println( falseMaxAve );
                                        Serial.println("threshold of magnification between falseMax and falseMin: "); Serial.println( slitThd );
                                 }
                        }
                }                
        }

        // 
        if( initBool == false ){
                if( encVal2_cr != encVal2_prv ){
                        if ( encVal2_cr == false ){
                                falseSrt = micros();
                                trueEnd = micros(); 
                                trueTime = trueEnd - trueSrt;                 
                                Serial.print( "true: " );          
                                Serial.println( trueTime ); 
                        }
                        if( encVal2_cr == true ){
                                trueSrt = micros();
                                falseEnd = micros();
                                falseTime = falseEnd - falseSrt;
                                if( falseTime * slitThd > falseMaxAve ){
                                        slitCount = 0;        
                                }else{
                                        slitCount += 1;
                                        digitalWrite(PIN_LED, HIGH);
                                        delay(1);
                                        digitalWrite(PIN_LED, LOW);                                        
                                }                        
                                Serial.print( "slit: " ); Serial.print( slitCount ); Serial.print( ", " );
                                Serial.print( "false: " ); Serial.print( falseTime ); Serial.print( ", " );
                        }
                }
        }
        encVal2_prv = encVal2_cr;

//        Serial.print(encVal2_cr);
//        Serial.print(",");
//        Serial.println(encVal1_cr);
        
//        Serial.print(encVal2_cr);
//        Serial.print(", ");
//        Serial.print(slitCount);
//        Serial.print(", ");
//        Serial.println(rotCount);
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

float exOutMean ( unsigned long x[], float xstdev, int xlength ){
        // except outliers mean
        unsigned long sum = 0;
        float ave = mean( x, xlength );
        float exOutAve = 0;
        int n = 0;
        for( int i = 0; i < xlength; i++ ){
                if( abs( (float)x[ i ] - ave ) <= 2*xstdev ){
                        sum += x[ i ]; 
                        n += 1;                                                                 
                }
        }
        return ave = (float)sum / n ;
}
