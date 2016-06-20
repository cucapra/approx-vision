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
    model_dir     = '../camera_models/NikonD7000/';

    % Image directory
    image_dir     = '../imgs/NikonD7000FL/';
    %image_dir     = '../imgs/';
    
    % Input image
    in_image_name = 'DSC_0906.NEF.raw_1C.tiff';
    %in_image_name = 'flower.NEF.raw_1C.tiff';
    
    % Reference
    ref_image_name = 'DSC_0906.JPG';
    %ref_image_name = 'flower.TIF';

    ForwardPipe(model_dir, image_dir, in_image_name, ref_image_name);
    
    
end



function ForwardPipe(model_dir, image_dir, in_image_name, ref_image_name)

    % Define patch size for analysis
    ystart = 2000;
    yend   = 2200;
    xstart =  500;
    xend   =  700;

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
        'Transform multiplication not equal to result found in model file' ) 

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
    in_scaled         = in_image * 8;
    %%%%%%% NOTE: D7000 has 14 bits not 12,
    %in_scaled        = in_image * 4;

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
  
    % Apply color space transform and white balance transform

    % Pre-allocate memory
    height           = size(image_float,1);
    width            = size(image_float,2);
    transformed      = zeros(height,width,3);
    gamutmapped      = zeros(height,width,3);
    tonemapped       = zeros(height,width,3);
    
    
    for y = 1:height
        for x = 1:width 
            
            % transformed = RAWdemosaiced * Ts * Tw
            transformed(y,x,:) = transpose(squeeze(image_float(y,x,:))) * TsTw;

            % gamut mapping
            gamutmapped(y,x,:) = h(squeeze(transformed(y,x,:)), ...
                ctrl_points, weights, c);
            
            % tone mapping
            tonemapped(y,x,:)  = tonemap(im2uint8(squeeze(gamutmapped(y,x,:))), frev);
            
        end
        % Let user know how far along we are
        disp((y/size(image_float,1))*100)
    end

    
    %==============================================================
    % Export Image(s)
    
    ref_image   = im2uint16(ref_image);
    image_float = im2uint16(image_float);
    transformed = im2uint16(transformed);
    gamutmapped = im2uint16(gamutmapped);
    tonemapped  = im2uint16(tonemapped);
    imwrite(ref_image,  strcat(image_dir,in_image_name,'.1.ref.tif'));
    imwrite(image_float,strcat(image_dir,in_image_name,'.2.demosaic.tif'));
    imwrite(transformed,strcat(image_dir,in_image_name,'.3.tranfmed.tif'));
    imwrite(gamutmapped,strcat(image_dir,in_image_name,'.4.gamut.tif'));
    imwrite(tonemapped, strcat(image_dir,in_image_name,'.5.toned.tif'));

end

% Gamut mapping function
function out = h (in, ctrl_points, weights, c)

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
function out = tonemap (in, f)

    out = zeros(3,1);

    for color = 1:3 % 1-R, 2-G, 3-B
        % Find index of value which is closest to the input
        [~,idx] = min(abs(f(:,color)-im2double(in(color))));
        
        % Convert the index to float representation of image value
        out(color) = (idx+1)/256;
    end

end