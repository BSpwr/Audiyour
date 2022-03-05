function filters = gen_equalizer_filters(Fs, num_stages)
    % gen_equalizer_filters -- Generate FIR filters for equalizer.

    % http://www.sengpielaudio.com/calculator-cutoffFrequencies.htm

    filter_order = num_stages * 2;

    % 1 octave
    Q = 1.414; % A Q factor of 1.414 will give a bandwidth of 1 octave
    center_freq = [31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000];

    for c = 1:length(center_freq)
        upper_cutoff = center_freq(c) * (sqrt(1 + 1 / (4 * (Q^2))) + (1 / (2 * Q)));
        lower_cutoff = center_freq(c) * (sqrt(1 + 1 / (4 * (Q^2))) - (1 / (2 * Q)));

        % clamp if needed
        if upper_cutoff > Fs / 2
            upper_cutoff = (Fs - 1) / 2;
        end

        if c == 1
            filters(c) = designfilt('lowpassiir', ...
                'HalfPowerFrequency', upper_cutoff, ...
                'FilterOrder', filter_order, ...
                'DesignMethod', 'butter', ...
                'SampleRate', Fs);
        else
            filters(c) = designfilt('bandpassiir', ...
                'HalfPowerFrequency1', lower_cutoff, ...
                'HalfPowerFrequency2', upper_cutoff, ...
                'FilterOrder', filter_order, ...
                'DesignMethod', 'butter', ...
                'SampleRate', Fs);
        end

    end

end
