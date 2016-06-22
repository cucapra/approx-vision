%==============================================================
% Image Processing Pipeline
% 
% This is a Matlab implementation of a pre-learned image 
% processing model. A description of the model can be found in
% "A New In-Camera Imaging Model for Color Computer Vision 
% and its Application" by Seon Joo Kim, Hai Ting Lin, 
% Michael Brown, et al. Code for learning a new model can 
% be found at the original project page.
%
% Original Project Page:
% http://www.comp.nus.edu.sg/~brown/radiometric_calibration/
%
% Model Format Readme:
% http://www.comp.nus.edu.sg/~brown/radiometric_calibration/datasets/Model_param/readme.pdf
% 
%==============================================================

function ImgPipe_Matlab
    % Model directory
    model_dir      = '../camera_models/NikonD7000/';

    % Image directory
    image_dir      = '../imgs/NikonD7000FL/';
    %image_dir     = '../imgs/';
    
    % Raw image
    raw_image_name  = 'DSC_0916.NEF.raw_1C.tiff';
    %in_image_name = 'flower.NEF.raw_1C.tiff';
    
    % Jpg image
    jpg_image_name = 'DSC_0916.JPG';
    %ref_image_name = 'flower.TIF';
    
    % Output data file
    out_data_name  = 'pipeline_data.txt';
    
    % Patch start locations
    %   [xstart,ystart]
    patchstarts = [ ...
        [550,  2750]; ... % 1
        [1000, 2750]; ... % 2
        [1500, 2750]; ... % 3
        [2000, 2750]; ... % 4
        [550,  2250]; ... % 5
        [1000, 2250]; ... % 6
        [1500, 2250]; ... % 7
        [2000, 2250]; ... % 8
    ];
    
    % Number of patch tests to run
    patchnum = 8;

    % Define patch size (patch width and height in pixels
    patchsize = 30;
    
    % Initialize results
    results  = zeros(patchnum,2,3);

    % Process patches
    for i=1:patchnum
    
        % Run the model on the patch
        [resultavg, refavg] = ForwardPipe(model_dir, image_dir, ...
            raw_image_name, jpg_image_name, ... 
            patchstarts(i,2), patchstarts(i,1), patchsize, i);
        
%         [resultavg, refavg] = BackwardPipe(model_dir, image_dir, ...
%             jpg_image_name, raw_image_name, ... 
%             patchstarts(i,2), patchstarts(i,1), patchsize, i);
        
        results(i,1,:) = resultavg;
        results(i,2,:) = refavg;
    
    end
    
    outfileID = fopen(out_data_name, 'w');
    
    % Display results
    fprintf(outfileID, 'res(red), res(green), res(blue)\n');
    fprintf(outfileID, 'ref(red), ref(green), ref(blue)\n');
    fprintf(outfileID, 'err(red), err(green), err(blue)\n');
    fprintf(outfileID, '\n');
    for i=1:patchnum
       fprintf(outfileID, 'Patch %d: \n', i);
       % Print results
       fprintf(outfileID, '%4.2f, %4.2f, %4.2f \n', ... 
           results(i,1,1), results(i,1,2), results(i,1,3));
       % Print reference
       fprintf(outfileID, '%4.2f, %4.2f, %4.2f \n', ... 
           results(i,2,1), results(i,2,2), results(i,2,3));
       % Print error
       fprintf(outfileID, '%4.2f, %4.2f, %4.2f \n', ... 
           geterror(results(i,1,1), results(i,2,1)), ...
           geterror(results(i,1,2), results(i,2,2)), ...
           geterror(results(i,1,3), results(i,2,3)));
       fprintf(outfileID, '\n');
    end
    
end



function [resultavg, refavg] = ForwardPipe(model_dir, image_dir, ...
    in_image_name, ref_image_name, ystart, xstart, patchsize, patchid)

    % Establish patch
    xend = xstart + patchsize;
    yend = ystart + patchsize;

    %==============================================================
    % Import Forward Model Data
    %
    % Note: This assumes a camera model folder with a single 
    % camera setting and transform. This is not the case for 
    % every folder, but it is for the Nikon D40 on the Normal
    % setting and with Fl(L14)/florescent color.

    % Model file reading
    transforms_file  = table2array( readtable( ...
        strcat(model_dir,'raw2jpg_transform.txt'), ...
        'Delimiter',' ','ReadVariableNames',false));
    ctrl_points_file = table2array( readtable( ...
        strcat(model_dir,'raw2jpg_ctrlPoints.txt'), ...
        'Delimiter',' ','ReadVariableNames',false));
    coeficients_file = table2array( readtable( ...
        strcat(model_dir,'raw2jpg_coefs.txt'), ...
        'Delimiter',' ','ReadVariableNames',false));
    resp_funct_file  = table2array( readtable( ...
        strcat(model_dir,'raw2jpg_respFcns.txt'), ...
        'Delimiter',' ','ReadVariableNames',false));

    % Color space transform
    Ts             = transforms_file(1:3,:);

    % White balance transform
    Tw             = diag(transforms_file(8,:));

    % Combined transforms
    TsTw           = Ts*Tw;
    TsTw_file      = transforms_file(5:7,:);

    % Perform quick check to determine equivalence with provided model
    % Round to nearest 4 decimal representation for check
    TsTw_4dec      = round(TsTw*10000)/10000;
    TsTw_file_4dec = round(TsTw_file*10000)/10000;
    assert( isequal( TsTw_4dec, TsTw_file_4dec), ...
        'Transform multiplication not equal to result found in model file, or import failed' ) 

    % Gamut mapping: Control points
    ctrl_points    = ctrl_points_file;

    % Gamut mapping: Weights
    weights        = coeficients_file(1:(size(coeficients_file,1)-4),:);

    % Gamut mapping: c
    c              = coeficients_file((size(coeficients_file,1)-3):end,:);

    % Tone mapping (reverse function is what is contained within model
    % file)
    frev           = resp_funct_file;

    %==============================================================
    % Import Raw Image Data

    % NOTE: Can use RAW2TIFF.cpp to convert raw to tiff. This isn't
    % automatically called by this script yet, but could be.

    in_image         = imread(strcat(image_dir,in_image_name));
    
    %==============================================================
    % Import Reference image
    
    ref_image        = imread(strcat(image_dir,ref_image_name));
    
    % Downsize to match patch size
    ref_image        = ref_image(ystart:yend,xstart:xend,:);
    
    %==============================================================
    % Forward pipeline function

    % Scale input image from 12 bit values to 16 bit values
    % (multiply by 2^4)
    %in_scaled         = in_image * 16;
    %%%%%%% NOTE: D7000 has 14 bits not 12
    % (multiply by 2^2)
    in_scaled        = in_image * 4;

    % Convert to uint16 representation for demosaicing
    in_image_unit16  = im2uint16(in_scaled);
    imwrite(in_image_unit16,strcat(image_dir,in_image_name,'.inimg.tif'));
    
    % Demosaic image
    demosaiced       = demosaic(in_image_unit16,'rggb');%gbrg %rggb 
    
    % Convert to double precision for transforming and gamut mapping
    image_float      = im2double(demosaiced);
    imwrite(image_float,strcat(image_dir,in_image_name,'.demosaicfull.tif'));
    
    % Downsize image for debugging
    image_float      = image_float(ystart:yend,xstart:xend,:);

    % Pre-allocate memory
    height           = size(image_float,1);
    width            = size(image_float,2);
    transformed      = zeros(height,width,3);
    gamutmapped      = zeros(height,width,3);
    tonemapped       = zeros(height,width,3);
    
    
    for y = 1:height
        for x = 1:width 
            
            % transformed = RAWdemosaiced * Ts * Tw
            transformed(y,x,:) = transpose(squeeze(image_float(y,x,:))) ...
                * transpose(TsTw);

            % gamut mapping
            gamutmapped(y,x,:) = RBF(squeeze(transformed(y,x,:)), ...
                ctrl_points, weights, c);
            
            % tone mapping
            tonemapped(y,x,:)  = tonemap(im2uint8(squeeze(gamutmapped(y,x,:))), frev);
            
        end
        % Let user know how far along we are
        disp((y/size(image_float,1))*100)
    end
    
    %==============================================================
    % Export Image(s)
    
    ref_image   = im2uint8(ref_image);
    image_float = im2uint8(image_float);
    transformed = im2uint8(transformed);
    gamutmapped = im2uint8(gamutmapped);
    tonemapped  = im2uint8(tonemapped);
%     imwrite(ref_image,  strcat(image_dir,in_image_name,'.ref.tif'));
%     imwrite(image_float,strcat(image_dir,in_image_name,'.demosaic.tif'));
%     imwrite(transformed,strcat(image_dir,in_image_name,'.tranfmed.tif'));
%     imwrite(gamutmapped,strcat(image_dir,in_image_name,'.gamut.tif'));
%     imwrite(tonemapped, strcat(image_dir,in_image_name,'.toned.tif'));

    imwrite(ref_image,  strcat(image_dir,in_image_name, ... 
        '.p',int2str(patchid),'.reference.tif'));
    imwrite(tonemapped,  strcat(image_dir,in_image_name, ... 
        '.p',int2str(patchid),'.result.tif'));
    
    %==============================================================
    % Produce pixel averages
    refavg    = zeros(3,1);
    resultavg = zeros(3,1);
    
    % Take two dimensional average
    for color = 1:3 % 1-R, 2-G, 3-B
        refavg(color)    = mean(mean(ref_image(:,:,color)));
        resultavg(color) = mean(mean(tonemapped(:,:,color)));
    end
    
end

function [resultavg, refavg] = BackwardPipe(model_dir, image_dir, ...
    in_image_name, ref_image_name, ystart, xstart, patchsize, patchid)

    % Establish patch
    xend = xstart + patchsize;
    yend = ystart + patchsize;

    %==============================================================
    % Import Backward Model Data
    %
    % Note: This assumes a camera model folder with a single 
    % camera setting and transform. This is not the case for 
    % every folder, but it is for the Nikon D40 on the Normal
    % setting and with Fl(L14)/florescent color.

    % Model file reading
    transforms_file  = table2array( readtable( ...
        strcat(model_dir,'jpg2raw_transform.txt'), ...
        'Delimiter',' ','ReadVariableNames',false));
    ctrl_points_file = table2array( readtable( ...
        strcat(model_dir,'jpg2raw_ctrlPoints.txt'), ...
        'Delimiter',' ','ReadVariableNames',false));
    coeficients_file = table2array( readtable( ...
        strcat(model_dir,'jpg2raw_coefs.txt'), ...
        'Delimiter',' ','ReadVariableNames',false));
    resp_funct_file  = table2array( readtable( ...
        strcat(model_dir,'jpg2raw_respFcns.txt'), ...
        'Delimiter',' ','ReadVariableNames',false));

    % Color space transform
    Ts             = transforms_file(1:3,:);

    % White balance transform
    Tw             = diag(transforms_file(8,:));

    % Combined transforms
    TsTw           = Ts*Tw;
    TsTw_file      = transforms_file(5:7,:);

    % Perform quick check to determine equivalence with provided model
    % Round to nearest 4 decimal representation for check
    TsTw_4dec      = round(TsTw*10000)/10000;
    TsTw_file_4dec = round(TsTw_file*10000)/10000;
    assert( isequal( TsTw_4dec, TsTw_file_4dec), ...
        'Transform multiplication not equal to result found in model file, or import failed' ) 
    
    % Reverse Gamut mapping: Control points
    ctrl_points    = ctrl_points_file;

    % Reverse Gamut mapping: Weights
    weights        = coeficients_file(1:(size(coeficients_file,1)-4),:);

    % Reverse Gamut mapping: c
    c              = coeficients_file((size(coeficients_file,1)-3):end,:);

    % Tone mapping (reverse function is what is contained within model
    % file)
    frev           = resp_funct_file;

    %==============================================================
    % Import Image Data

    in_image         = imread(strcat(image_dir,in_image_name));
    ref_image        = imread(strcat(image_dir,ref_image_name));
 
    % Scale reference image to 16 bits
    % Note: this currently assumes 14 bit raw reference
    % (multiply by 2^2)
    %ref_image        = im2double(ref_image);

    
    %==============================================================
    % Backward pipeline function
 
    % Convert to double precision for processing
    image_float      = im2double(in_image);
    
    % Extract patches
    image_float      = image_float(ystart:yend,xstart:xend,:);
    ref_image        = ref_image  (ystart:yend,xstart:xend);

    % Pre-allocate memory
    height           = size(image_float,1);
    width            = size(image_float,2);
    revtransformed      = zeros(height,width,3);
    revtonemapped       = zeros(height,width,3);
    revgamutmapped      = zeros(height,width,3);
    remosaiced          = zeros(height,width,3);
    ref_image_colored   = zeros(height,width,3);
    
    for y = 1:height
        for x = 1:width 
            
            % Reverse tone mapping
            revtonemapped(y,x,:)  = revtonemap(squeeze(image_float(y,x,:)), frev);
            
            % Reverse gamut mapping
            revgamutmapped(y,x,:) = RBF(squeeze(revtonemapped(y,x,:)), ...
                ctrl_points, weights, c);
            
            % Reverse color mapping and white balancing
            % RAWdemosaiced = transformed * inv(TsTw) = transformed / TsTw
            revtransformed(y,x,:) = transpose(squeeze(image_float(y,x,:))) ...
                / transpose(TsTw);
            
            % Re-mosaicing
            % Note: This is not currently parameterizable, assumes rggb
            yodd = mod(y,2);
            xodd = mod(x,2);
            % If a red pixel
            if yodd && xodd
                remosaiced(y,x,:) = [revtransformed(y,x,1), 0, 0];
            % If a green pixel
            elseif xor(yodd,xodd)
                remosaiced(y,x,:) = [0, revtransformed(y,x,2), 0];
            % If a blue pixel
            elseif ~yodd && ~xodd
                remosaiced(y,x,:) = [0, 0, revtransformed(y,x,3)];
            end
            
            %======================================================
            % Reorganize reference image
            % Note: This is not currently parameterizable, assumes rggb
            % If a red pixel
            if yodd && xodd
                ref_image_colored(y,x,:) = [ref_image(y,x), 0, 0];
            % If a green pixel
            elseif xor(yodd,xodd)
                ref_image_colored(y,x,:) = [0, ref_image(y,x), 0];
            % If a blue pixel
            elseif ~yodd && ~xodd
                ref_image_colored(y,x,:) = [0, 0, ref_image(y,x)];
            end
            
        end
        % Let user know how far along we are
        disp((y/size(image_float,1))*100)
    end
    
    
    %==============================================================
    % Export Image(s)
    
    ref_image_colored = im2uint8(ref_image_colored);
    remosaiced        = im2uint8(remosaiced);

    imwrite(ref_image_colored,  strcat(image_dir,in_image_name, ... 
        '.p',int2str(patchid),'.reference.tif'));
    imwrite(remosaiced,  strcat(image_dir,in_image_name, ... 
        '.p',int2str(patchid),'.result.tif'));
    
    %==============================================================
    % Produce pixel averages
    refavg    = zeros(3,1);
    resultavg = zeros(3,1);
    
    % Take two dimensional average
    for color = 1:3 % 1-R, 2-G, 3-B
        refavg(color)    = mean(mean(ref_image_colored(:,:,color)));
        resultavg(color) = mean(mean(remosaiced(:,:,color)));
    end
    
end


% Gamut mapping function
function out = RBF (in, ctrl_points, weights, c)

    out      = zeros(3,1);

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Weighted control points
    for idx = 1:size(ctrl_points,1)
        dist = norm(transpose(in) - ctrl_points(idx,:));
        for color = 1:3
            out(color)  = out(color) + weights(idx,color) * dist;
        end
    end
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Biases
    for color = 1:3
        out(color) = out(color) +  c(1,color);
        out(color) = out(color) + (c(2,color) * in(1));
        out(color) = out(color) + (c(3,color) * in(2));
        out(color) = out(color) + (c(4,color) * in(3));
    end
    
end

% Tone mapping function
function out = tonemap (in, revf)

    out = zeros(3,1);

    for color = 1:3 % 1-R, 2-G, 3-B
        % Find index of value which is closest to the input
        [~,idx] = min(abs(revf(:,color)-im2double(in(color))));
        
        % Convert the index to float representation of image value
        out(color) = (idx+1)/256;
    end

end

% Tone mapping function
function out = revtonemap (in, revf)

    out = zeros(3,1);

    for color = 1:3 % 1-R, 2-G, 3-B
        % Convert the input to an integer between 1 and 256
        idx = round(in(color)*256);
        
        % If index is zero, bump up to 1 to prevent 0 indexing in Matlab
        if idx == 0
           idx = 1; 
        end
        
        % Index the reverse tone mapping function
        out(color) = revf(idx,color);        
    end

end

% Error computation function
function error = geterror(result, reference)

    diff  = result-reference;
    error = (diff/reference)*100;

end