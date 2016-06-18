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
    model_dir     = '../camera_models/NikonD40/Fl(L14)/';

    % Image directory
    image_dir     = '../imgs/';

    % Input image
    in_image_name = 'beer_hall.NEF.raw_1C.tiff';

    ForwardPipe(model_dir, image_dir, in_image_name);
    
end



function ForwardPipe(model_dir, image_dir, in_image_name)
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
    c              = transpose(coeficients_file((size(coeficients_file,1)-3):end,:));

    % Tone mapping
    f              = resp_funct_file;

    %==============================================================
    % Import Raw Image Data

    % NOTE: Can use RAW2TIFF.cpp to convert raw to tiff. This isn't
    % automatically called by this script yet, but could be.

    in_image = imread(strcat(image_dir,in_image_name));

    %==============================================================
    % Forward pipeline function

    % Scale input image from 12 bit values to 16 bit values
    % (multiply by 2^4)
    in_scaled        = in_image * 16;

    % Convert to uint16 representation
    in_image_unit16  = im2uint16(in_scaled);

    % Demosaic image
    demosaiced       = demosaic(in_image_unit16,'gbrg');
    
    % Print to image for comparison
    imwrite(demosaiced,strcat(image_dir,in_image_name,'.demosaic.tif'));
  
    % Cast to float for rest of processing
    image_float      = im2double(demosaiced);

    % Apply color space transform and white balance transform

    % Pre-allocate memory
    height           = size(image_float,1);
    width            = size(image_float,2);
    transformed      = zeros(height,width,3);
    gamutmapped      = zeros(height,width,3);
    tonemapped       = zeros(height,width,3);
    
    % Debugging code
%      image_float(1,1,:) = transpose([1,1,1]);
%      transpose(squeeze(image_float(1,1,:)))
%      [1,1,1]*TsTw
%      transformed(1,1,:) = transpose(squeeze(image_float(1,1,:))) * TsTw;
%      transpose(squeeze(transformed(1,1,:)))
    
    % Better way to do this?
    for y = 1:height
        for x = 1:(width) 
            % transformed = RAWdemosaiced * Ts * Tw

            transformed(y,x,:) = transpose(squeeze(image_float(y,x,:))) * TsTw;

            % gamut mapping
            %gamutmapped(y,x,:) = h(squeeze(transformed(y,x,:)), ...
            %    ctrl_points, weights, c);
            
        end
        % Let user know how far along we are
        disp((y/size(image_float,1))*100)
    end

    
    % Tone Mapping
%     tonemapped         = im2uint8(gamutmapped);
%     f                  = im2uint8(f);
%     for y = 1:height
%         for x = 1:(width/2) % Only process left half for debugging
%             for color = 1:3 % 1-R, 2-G, 3-B
%                 %y
%                 %x
%                 %color
%                 %tonemapped(y,x,color)
%                 %f(tonemapped(y,x,color),color)
%                 
%                 % Correct index if the value at the pixel is zero
%                 if tonemapped(y,x,color) == 0 
%                     index = 1;
%                 else
%                     index = tonemapped(y,x,color);
%                 end
%                 % Map tone
%                 tonemapped(y,x,color) = f(index,color);
%             end
%         end
%     end
    
    
    % Prepare output
    out_image = im2uint8(transformed);
    
    
    %==============================================================
    % Export Image

    imwrite(out_image,strcat(image_dir,in_image_name,'.output.tif'));
end


function out = h (in, ctrl_points, weights, c)

    out      = zeros(1,3);
    in_tilda = [1,transpose(in)];

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Biases
    for color = 1:3
        out(color) = dot(in_tilda,c(color,:));
    end
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Weighted control points
    for idx = 1:size(ctrl_points,1)
        dist = in - ctrl_points(idx);
        out  = out + dot(weights(idx,:),dist);
    end
    
end