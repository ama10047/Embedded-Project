// If you see error here, please do following instructions:
// 1. go to PIO HOME -> Library
// 2. Search for "Adafruit Circuit Playground" and add it to the project
#include <Adafruit_CircuitPlayground.h>

// Define sample size and tolerance thresholds for movement comparison
#define SAMPLE_SIZE 50         // Number of samples to record for each movement sequence
#define TOLERANCE_HIGH 7       // Upper threshold for movement difference (no score if exceeded)
#define TOLERANCE_LOW 4        // Lower threshold for movement difference (full score if below)
#define SUCCESS_THRESHOLD 0.90 // Required match percentage for successful unlock (90%)

// Structure to store movement data for all three axes
struct Movement
{
    float x[SAMPLE_SIZE]; // X-axis acceleration samples
    float y[SAMPLE_SIZE]; // Y-axis acceleration samples
    float z[SAMPLE_SIZE]; // Z-axis acceleration samples
};

Movement lockMovement; // Stores the recorded "key" movement pattern
Movement testMovement; // Stores the attempted unlock movement pattern
bool isLocked = false; // System state flag

// Function prototypes
void recordMovement(Movement &movement);
float compareMovements(Movement &arr1, Movement &arr2);
void blinkLED(uint8_t r, uint8_t g, uint8_t b, int duration);

// Setup function for setting up necessary Pins on the board
void setup()
{
    CircuitPlayground.begin();
    CircuitPlayground.setBrightness(50);

    // Configure input buttons
    pinMode(4, INPUT_PULLUP);  // Left button
    pinMode(19, INPUT_PULLUP); // Right button

    CircuitPlayground.clearPixels();
}

// Loop function to run program on the board
void loop()
{
    // Read button states
    bool button1 = digitalRead(4);  // Left button
    bool button2 = digitalRead(19); // Right button

    // System reset: Both buttons pressed at the same time
    // The test button will be turned off. 
    // It will be unlocked again after record a movement
    if (button1 && button2)
    {
        isLocked = false;
        blinkLED(128, 0, 128, 1500); // Purple: System reset
    }
    // Record mode: click on the Left button on the board.
    // After you see green light blinks, do the movement.
    // When you finish movement, stop until you see blue LED blinks
    else if (button1)
    {
        blinkLED(0, 255, 0, 100); // Green: Start recording
        recordMovement(lockMovement);
        isLocked = true;
        blinkLED(0, 0, 255, 100); // Blue: Lock successful
    }
    // Unlock attempt: Right button when system is locked
    // Similar to record button, click on the test button.
    // After you see yellow light blinks, do the movement.
    // When you finish movement, stop until you see Red/Green LED turned on
    else if (button2 && isLocked)
    {
        blinkLED(255, 255, 0, 100); // Yellow: Start unlock attempt
        recordMovement(testMovement);

        float score = compareMovements(testMovement, lockMovement);

        if (score >= SUCCESS_THRESHOLD)
        {
            // Green: Unlock successful, meaning successfully replicate recorded movement
            blinkLED(0, 255, 0, 3000); 
        }
        else
        {
            // Red: Unlock failed, meaning fail to replicate recorded movement
            blinkLED(255, 0, 0, 3000); 
        }
    }

    delay(100); // Debounce delay for buttons
}

// Records acceleration data for a movement sequence
void recordMovement(Movement &movement)
{
    for (int i = 0; i < SAMPLE_SIZE; i++)
    {
        // Sample all three axes of acceleration
        movement.x[i] = CircuitPlayground.motionX();
        movement.y[i] = CircuitPlayground.motionY();
        movement.z[i] = CircuitPlayground.motionZ();
        delay(100); // 100ms sampling interval for stable readings
    }
    return;
}

// Compares two movement sequences using moving average and tolerance-based scoring
float compareMovements(Movement &arr1, Movement &arr2)
{
    float score = 0;
    float dispDiff = 0; // Tracks total displacement difference for additional validation

    for (int i = 0; i < SAMPLE_SIZE; i++)
    {
        // Calculate moving averages with special handling for edge cases
        float average1x, average2x, average1y, average2y, average1z, average2z;

        // Edge case: Start of sequence (first 3 samples)
        if (i == 0)
        {
            average1x = ((arr1.x[0]) + (arr1.x[1]) + (arr1.x[2])) / 3;
            average2x = ((arr2.x[0]) + (arr2.x[1]) + (arr2.x[2])) / 3;
            average1y = ((arr1.y[0]) + (arr1.y[1]) + (arr1.y[2])) / 3;
            average2y = ((arr2.y[0]) + (arr2.y[1]) + (arr2.y[2])) / 3;
            average1z = ((arr1.z[0]) + (arr1.z[1]) + (arr1.z[2])) / 3;
            average2z = ((arr2.z[0]) + (arr2.z[1]) + (arr2.z[2])) / 3;
        }
        // Edge case: Second position (4 samples)
        else if (i == 1)
        {
            average1x = ((arr1.x[0]) + (arr1.x[1]) + (arr1.x[2]) + (arr1.x[3])) / 4;
            average2x = ((arr2.x[0]) + (arr2.x[1]) + (arr2.x[2]) + (arr2.x[3])) / 4;
            average1y = ((arr1.y[0]) + (arr1.y[1]) + (arr1.y[2]) + (arr1.y[3])) / 4;
            average2y = ((arr2.y[0]) + (arr2.y[1]) + (arr2.y[2]) + (arr2.y[3])) / 4;
            average1z = ((arr1.z[0]) + (arr1.z[1]) + (arr1.z[2]) + (arr1.z[3])) / 4;
            average2z = ((arr2.z[0]) + (arr2.z[1]) + (arr2.z[2]) + (arr2.z[3])) / 4;
        }
        // Edge case: Second-to-last position
        else if (i == 48)
        {
            average1x = ((arr1.x[46]) + (arr1.x[47]) + (arr1.x[48]) + (arr1.x[49])) / 4;
            average2x = ((arr2.x[46]) + (arr2.x[47]) + (arr2.x[48]) + (arr2.x[49])) / 4;
            average1y = ((arr1.y[46]) + (arr1.y[47]) + (arr1.y[48]) + (arr1.y[49])) / 4;
            average2y = ((arr2.y[46]) + (arr2.y[47]) + (arr2.y[48]) + (arr2.y[49])) / 4;
            average1z = ((arr1.z[46]) + (arr1.z[47]) + (arr1.z[48]) + (arr1.z[49])) / 4;
            average2z = ((arr2.z[46]) + (arr2.z[47]) + (arr2.z[48]) + (arr2.z[49])) / 4;
        }
        // Edge case: Last position (last 3 samples)
        else if (i == 49)
        {
            average1x = ((arr1.x[47]) + (arr1.x[48]) + (arr1.x[49])) / 3;
            average2x = ((arr2.x[47]) + (arr2.x[48]) + (arr2.x[49])) / 3;
            average1y = ((arr1.y[47]) + (arr1.y[48]) + (arr1.y[49])) / 3;
            average2y = ((arr2.y[47]) + (arr2.y[48]) + (arr2.y[49])) / 3;
            average1z = ((arr1.z[47]) + (arr1.z[48]) + (arr1.z[49])) / 3;
            average2z = ((arr2.z[47]) + (arr2.z[48]) + (arr2.z[49])) / 3;
        }
        // Standard case: 5-point moving average for middle positions
        else
        {
            average1x = ((arr1.x[i]) + (arr1.x[i - 1]) + (arr1.x[i - 2]) + (arr1.x[i + 1]) + (arr1.x[i + 2])) / 5;
            average2x = ((arr2.x[i]) + (arr2.x[i - 1]) + (arr2.x[i - 2]) + (arr2.x[i + 1]) + (arr2.x[i + 2])) / 5;
            average1y = ((arr1.y[i]) + (arr1.y[i - 1]) + (arr1.y[i - 2]) + (arr1.y[i + 1]) + (arr1.y[i + 2])) / 5;
            average2y = ((arr2.y[i]) + (arr2.y[i - 1]) + (arr2.y[i - 2]) + (arr2.y[i + 1]) + (arr2.y[i + 2])) / 5;
            average1z = ((arr1.z[i]) + (arr1.z[i - 1]) + (arr1.z[i - 2]) + (arr1.z[i + 1]) + (arr1.z[i + 2])) / 5;
            average2z = ((arr2.z[i]) + (arr2.z[i - 1]) + (arr2.z[i - 2]) + (arr2.z[i + 1]) + (arr2.z[i + 2])) / 5;
        }

        // Calculate total difference between movements at this point
        float num = abs(average1x - average2x) +
                    abs(average1y - average2y) +
                    abs(average1z - average2z);
        dispDiff = dispDiff + num;

        // Score based on difference thresholds
        if (num > TOLERANCE_HIGH)
        {
            // No score for large differences
        }
        else if (num <= TOLERANCE_LOW)
        {
            score += 1; // Full point for close match
        }
        else
        {
            score += 0.5; // Half point for partial match
        }
    }

    // Debug output
    Serial.println(dispDiff);
    Serial.println(score / SAMPLE_SIZE);

    // Return success (1) only if both displacement difference and match score meet criteria
    if ((dispDiff < 110) && ((score / SAMPLE_SIZE) > SUCCESS_THRESHOLD))
    {
        return 1;
    }
    return 0;
}

// Provides LED feedback for different system states
void blinkLED(uint8_t r, uint8_t g, uint8_t b, int duration)
{
    CircuitPlayground.setPixelColor(0, r, g, b);
    delay(duration);
    CircuitPlayground.clearPixels();
}