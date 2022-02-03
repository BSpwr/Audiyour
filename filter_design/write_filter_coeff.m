function write_filter_coeff(fileName, filters)
    % write_filter_coeff - writes coeffecients of IIR filter to file as C arrays
    %

    fileID = fopen(fileName, 'w');

    buff_len = 0;

    for i = 1:length(filters)
        f_size = size(filters(i).Coefficients);
        buff_len = buff_len + f_size(1) * 4;
    end

    f1_size = size(filters(1).Coefficients);

    fprintf(fileID, '#define EQ_NUM_BANDS %d\n', size(filters, 2));
    fprintf(fileID, '#define IIR_FILTER_NUM_STAGES %d\n', f1_size(1));
    fprintf(fileID, 'typedef float dfi_iir_filter[IIR_FILTER_NUM_STAGES][%d];\n', f1_size(2));
    fprintf(fileID, 'typedef float dfi_iir_filter_buffer[IIR_FILTER_NUM_STAGES][4];\n\n');
    fprintf(fileID, 'extern const dfi_iir_filter g_equalizer_filters[EQ_NUM_BANDS];\n\n\n');

    fprintf(fileID, 'const dfi_iir_filter g_equalizer_filters[EQ_NUM_BANDS] = {\n');

    for i = 1:length(filters)
        coeff = filters(i).Coefficients;
        f_size = size(coeff);
        fprintf(fileID, '{');

        for j = 1:f_size(1)

            if j == 1
                fprintf(fileID, '{%f, %f, %f, %f, %f, %f}', coeff(j, :));
            else
                fprintf(fileID, '                               {%f, %f, %f, %f, %f, %f}', coeff(j, :));
            end

            if j ~= f_size(1)
                fprintf(fileID, ',\n');
            end

        end

        
        if i ~= length(filters)
            fprintf(fileID, '},\n\n');
        else
            fprintf(fileID, '}\n\n');
        end
    end
     
    fprintf(fileID, '};\n\n');

end
