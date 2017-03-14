#include "Tripwire.h"

Tripwire::Tripwire(range_function get_range) {
    /**
    * Create a new tripwire object
    * @param get_range Passed function to get the range of the function. Unitless,
    * but must be a long.
    */
    if (get_range != NULL) {
        _get_range = get_range;
    }

    is_calibrated = false;
    distance = 0;
    baseline_distance = 0;
    baseline_variance = 0;
    distance_threshold = DEFAULT_DISTANCE_THRESHOLD;
    min_baseline_reads = DEFAULT_MIN_BASELINE_READS;
    max_baseline_reads = DEFAULT_MAX_BASELINE_READS;
    max_baseline_variance = DEFAULT_MAX_BASELINE_VARIANCE;
    baseline_read_interval = DEFAULT_BASELINE_READ_INTERVAL;
    min_successive_detections = DEFAULT_MINIMUM_SUCCESSIVE_DETECTIONS;
}

void Tripwire::start() {
    /**
    * Start the tripwire.
    * Runtime variables are reset.
    * update must be called regularly to ensure consistent reads
    */
    event_start_time = 0;
    last_event_width = 0;
    _successive_detections = 0;
    num_detections = 0;

    calibrate();
}

void Tripwire::calibrate() {
    /**
    * Calibrate the baseline distance of the sensor.
    * Ensure nothing is moving in front of the sensor during calibration or the
    * baseline will be inaccurate.
    * is_calibrated will be updated after this function has completed to indicate
    * whether the calibration was successful.
    */
    is_calibrated = false;

    if (_get_range && distance_threshold) {
        baseline_distance = (*_get_range)();
        baseline_variance = baseline_distance;
        int baseline_reads = 1;

        // Keep reading in the baseline until it stablises or until the max reads are reached
        while ((baseline_reads < min_baseline_reads || baseline_variance > max_baseline_variance) &&
               baseline_reads < max_baseline_reads) {
            long new_range = (*_get_range)();
            long new_variance = abs(baseline_distance - new_range);

            baseline_variance = ((baseline_variance + new_variance) / 2);
            baseline_distance = ((baseline_distance + new_range) / 2);

            baseline_reads++;
            delay(baseline_read_interval);
        }

        // Calibration fails if the range is varying too much
        if (baseline_variance < max_baseline_variance) {
            is_calibrated = true;
        }
    }
}

void Tripwire::update() {
    /**
    * Update the state of the tripwire
    * This function should be called regularly to ensure data is accurate.
    * Event callbacks are called from this function.
    */
    distance = (*_get_range)();

    // Detection occurs when target breaks the LoS to the baseline
    if ((baseline_distance - distance) > distance_threshold) {
        // An event starts if the minimum number of successive detections has been
        // reached. Only one event is recorded per "trip"
        if (_successive_detections == min_successive_detections) {
            _successive_detections += 1;
            event_start_time = millis();
            num_detections++;

            // Call the event start callback if it's been set
            if (_event_start) {
                (*_event_start)();
            }
        }

        // Not enough successive detections yet
        else if (_successive_detections < min_successive_detections) {
            _successive_detections += 1;
        }
    }

    // Nothing detected. End any active events and reset tripwire.
    else {
        if (_successive_detections > 0) {
            last_event_width = millis() - event_start_time;

            // Call the event end callback if it has been set
            if (_event_end) {
                (*_event_end)();
            }
        }
        _successive_detections = 0;
    }
}

void Tripwire::set_range_function(range_function function) {
    /**
    * Set the _get_range function
    * @param function The external function that returns the recorded distance of
    * the sensor
    */
    if (function != NULL) {
        _get_range = function;
    }
}

void Tripwire::set_event_start_callback(event_callback callback) {
    /**
    * Set the event start callback function
    * @param callback Function to call when an event starts
    */
    if (callback != NULL) {
        _event_start = callback;
    }
}

void Tripwire::set_event_end_callback(event_callback callback) {
    /**
    * Set the event end callback function
    * @param callback Function to call when an event ends
    */
    if (callback != NULL) {
        _event_end = callback;
    }
}

void Tripwire::reset_event_status() {
    /**
    * Reset the event status of the tripwire.
    * This is a manual way of allowing new events to be detected in case something gets stuck.
    */
    _successive_detections = 0;
}
