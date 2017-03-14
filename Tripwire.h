#ifndef TRIPWIRE_H
#define TRIPWIRE_H

#include <Arduino.h>

typedef long (*range_function)(void); /**< Passed function to get the range of the sensor */
typedef void (*event_callback)(void); /**< Callback function for sensor events */

class Tripwire {
    /**
    * Library to manage detection events for a ranging sensor
    */

   public:
    /**
    * Create a new tripwire object
    * @param get_range Passed function to get the range of the function. Unitless,
    * but must be a long.
    */
    Tripwire(range_function get_range = NULL);

    /**
    * Start the tripwire.
    * Runtime variables are reset.
    * update must be called regularly to ensure consistent reads
    * @param distance_threshold Minimum reduction in distance to count a
    * detection. Units are inhereted from the _get_range function.
    */
    void start();

    /**
    * Calibrate the baseline distance of the sensor.
    * Ensure nothing is moving in front of the sensor during calibration or the
    * baseline will be inaccurate.
    * is_calibrated will be updated after this function has completed to indicate
    * whether the calibration was successful.
    */
    void calibrate();

    /**
    * Update the state of the tripwire
    * This function should be called regularly to ensure data is accurate.
    * Event callbacks are called from this function.
    */
    void update();

    /**
    * Set the _get_range function
    * @param function The external function that returns the recorded distance of
    * the sensor
    */
    void set_range_function(range_function function);

    /**
    * Set the event start callback function
    * @param callback Function to call when an event starts
    */
    void set_event_start_callback(event_callback callback);

    /**
    * Set the event end callback function
    * @param callback Function to call when an event ends
    */
    void set_event_end_callback(event_callback callback);

    /**
    * Reset the event status of the tripwire.
    * This is a manual way of allowing new events to be detected in case something gets stuck.
    */
    void reset_event_status();

    bool is_calibrated;             /**< Indicator that the sensor is calibrated. True if
                                       calibrated*/
    unsigned long num_detections;   /**< Number of events detected by the sensor */
    long distance;                  /**< Last distance measured by the sensor. Units inhereted */
    long baseline_distance;         /**< Background range of the sensor */
    long baseline_variance;         /**< Amount that the baseline varied during
                                       calibration. Units inhereted */
    long distance_threshold;        /**< Amount the distance must change to count as a
                                       detection. Units inhereted. */
    int min_baseline_reads;         /**< The minimum number of baseline reads needed for
                                       calibration */
    int max_baseline_reads;         /**< The maximum number of baseline reads needed for
                                       calibration */
    long max_baseline_variance;     /**< The maximum allowed baseline variance.
                                       Calibration will fail if the variance exceeds
                                       this value. */
    long baseline_read_interval;    /**< Amount of time between sensor reads during
                                       calibration. [ms] */
    int min_successive_detections;  /**< Minimum number of successive detects
                                       needed to trigger an event. */
    unsigned long last_event_width; /**< Width of the last sensor event [ms] */
    unsigned long event_start_time; /**< Time that the last sensor event started [ms] */

   private:
    event_callback _event_start; /**< Function to call when an event starts */
    event_callback _event_end;   /**< Function to call when an event ends */
    range_function _get_range;   /**< External function that returns the sensor's
                                    reported distance */
    int _successive_detections;  /**< The current number of successive detections
                                    */

    const static long DEFAULT_DISTANCE_THRESHOLD = 70; /**< Default threshold distance [implied cm] */
    const static int DEFAULT_MIN_BASELINE_READS = 20;  /**< Default minimum number of baseline reads for calibration */
    const static int DEFAULT_MAX_BASELINE_READS = 40;  /**< Default maximum number of baseline reads for calibration */
    const static long DEFAULT_MAX_BASELINE_VARIANCE =
        DEFAULT_DISTANCE_THRESHOLD; /**< Default maximum allowed variance in the
                                       baseline reading [cm] */
    const static long DEFAULT_BASELINE_READ_INTERVAL =
        100; /**< Default time between sensor reads during calibration [ms] */
    const static int DEFAULT_MINIMUM_SUCCESSIVE_DETECTIONS = 0; /**< Default number of successive sensor detections */
};
#endif
